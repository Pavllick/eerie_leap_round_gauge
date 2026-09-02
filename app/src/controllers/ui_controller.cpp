#include <cerrno>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include <zephyr/logging/log.h>
#include <eerie_memory.hpp>

#include "utilities/memory/memory_resource_manager.h"

#include "configuration/cbor/cbor_ui_config/cbor_ui_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "domain/ui_domain/event_bus/navigation_event_channel.h"
#include "domain/ui_domain/event_bus/ui_signal_type.h"
#include "domain/ui_domain/lvgl_lock.h"
#include "domain/ui_domain/models/navigation_intent.h"
#include "domain/ui_domain/models/widget_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/models/icon_type.h"
#include "domain/ui_domain/models/indicator_direction.h"
#include "domain/ui_domain/models/indicator_value_source.h"
#include "domain/settings_domain/models/setting_id.h"

#include "views/screens/screen_factory.h"
#include "views/themes/theme_manager.h"
#include "views/themes/dark_bw_theme.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"
#include "views/assets/images/images_register.h"

#include "ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::screens;
using namespace eerie_leap::views::themes;
using namespace eerie_leap::views::widgets::indicators;
using namespace eerie_leap::views::assets::images;

namespace config_services = eerie_leap::configuration::services;

using eerie_leap::domain::ui_domain::ScopedLvglLock;
using eerie_leap::domain::ui_domain::event_bus::NavigationEventType;
using eerie_leap::domain::ui_domain::event_bus::NavigationPayloadType;
using eerie_leap::domain::ui_domain::event_bus::UiSignalType;
using eerie_leap::subsys::event_bus::CreateScopedSubscription;
using eerie_leap::domain::settings_domain::models::SettingId;

LOG_MODULE_REGISTER(ui_controller_logger);

UiController::UiController(
    std::shared_ptr<IFsService> fs_service,
    std::shared_ptr<WorkQueueThread> config_work_queue_thread,
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SettingsRegistry> settings_registry,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame)
        : fs_service_(std::move(fs_service)),
        config_work_queue_thread_(std::move(config_work_queue_thread)),
        configuration_service_(std::move(configuration_service)),
        settings_registry_(std::move(settings_registry)),
        sensor_readings_frame_(std::move(sensor_readings_frame)) {}

UiController::~UiController() {
    navigation_subscription_.reset();

    // The dispatcher holds a raw pointer to a work queue destroyed before main_view_.
    if(main_view_ != nullptr)
        main_view_->SetRenderDispatcher(nullptr);
}

int UiController::Initialize() {
    ThemeManager::GetInstance().SetTheme(std::make_shared<DarkBWTheme>());

    ui_renderer_service_ = std::make_shared<UiRendererService>();
    if(ui_renderer_service_->Initialize() != 0) {
        LOG_ERR("Failed to initialize the UI renderer service.");
        return -1;
    }

    ui_renderer_service_->Start();

    auto cbor_ui_config_service = std::make_unique<config_services::CborConfigurationService<CborUiConfig>>(
        UI_CONFIGURATION_NAME, fs_service_, config_work_queue_thread_);
    ui_configuration_manager_ = std::make_shared<UiConfigurationManager>(
        std::move(cbor_ui_config_service));

    if(configuration_service_ != nullptr)
        configuration_service_->RegisterCborConfigurationManager(
            ConfigurationService::Type::Ui, ui_configuration_manager_);

    ui_assets_manager_ = std::make_shared<AssetsManager>(fs_service_, UI_ASSETS_DIR);

    // TODO: For test purposes only
    SetupTestConfiguration();
    SetupTestAssets();

    ScopedLvglLock lvgl_guard;

    main_view_ = std::make_unique<MainView>();
    navigation_service_ = std::make_shared<NavigationService>();

    ui_render_work_queue_thread_ = std::make_unique<WorkQueueThread>(
        "ui_render_work_q",
        ui_render_work_queue_stack_size_,
        ui_render_work_queue_priority_);

    if(ui_render_work_queue_thread_->Initialize()) {
        // Keeps the event bus worker - and the LVGL lock it holds while
        // dispatching - out of the multi-frame first render of a screen group.
        main_view_->SetRenderDispatcher(
            [work_queue = ui_render_work_queue_thread_.get()](std::function<void()> work) {
                try {
                    work_queue->Run(std::move(work));
                } catch(const std::exception& e) {
                    LOG_ERR("Failed to queue a screen group render. %s", e.what());
                }
            });
    } else {
        LOG_ERR("Failed to start the UI render work queue, rendering inline.");
        ui_render_work_queue_thread_.reset();
    }

    widget_context_ = WidgetContext {
        .assets_manager = ui_assets_manager_,
        .settings_provider = settings_registry_,
        .navigation_service = navigation_service_
    };

    if(Configure(ui_configuration_manager_->Get()) != 0)
        LOG_ERR("Failed to configure the UI from the stored configuration.");

    if(sensor_readings_frame_ != nullptr) {
        sensors_rendering_service_ = std::make_shared<SensorsRenderingService>(sensor_readings_frame_);
        sensors_rendering_service_->Initialize();
    }

    SubscribeToNavigation();

    ui_input_service_ = std::make_shared<UiInputService>(navigation_service_);
    if(ui_input_service_->Initialize(main_view_->GetContainer()->GetObject()) != 0)
        LOG_ERR("Failed to initialize the UI input service.");

    return 0;
}

int UiController::Start() {
    int res = 0;

    {
        ScopedLvglLock lvgl_guard;
        res = main_view_->Render();
    }

    if(res != 0)
        return res;

    if(sensors_rendering_service_ != nullptr)
        sensors_rendering_service_->Start();

    return 0;
}

int UiController::Configure(std::shared_ptr<UiConfiguration> config) {
    configuration_ = std::move(config);

    // Screen and widget construction creates LVGL objects while the renderer
    // thread is already running.
    ScopedLvglLock lvgl_guard;

    for(auto& screen_config : configuration_->screen_configurations) {
        auto screen = CreateScreen(screen_config);

        if(screen != nullptr)
            main_view_->AddScreen(std::move(screen));
    }

    main_view_->PruneEmptyGroups();

    auto group_ids = main_view_->GetGroupIds();
    if(group_ids.empty()) {
        LOG_ERR("No usable screen groups were configured.");
        return -ENOENT;
    }

    int res = main_view_->SetActiveGroup(configuration_->active_screen_group_id);
    if(res != 0) {
        LOG_WRN("Falling back to screen group %u.", group_ids.front());
        res = main_view_->SetActiveGroup(group_ids.front());
    }

    navigation_service_->SetGroupIds(std::move(group_ids));
    if(auto active_group_id = main_view_->GetActiveGroupId())
        navigation_service_->SetActiveGroupId(*active_group_id);

    return res;
}

void UiController::SubscribeToNavigation() {
    navigation_subscription_ = CreateScopedSubscription(
        NavigationEventChannel::GetInstance(),
        NavigationEventType::Changed,
        [this](const NavigationEventChannel::EventMessage& event) { OnNavigationChanged(event); });

    if(navigation_subscription_ == nullptr)
        LOG_ERR("Failed to subscribe to navigation events.");
}

void UiController::OnNavigationChanged(const NavigationEventChannel::EventMessage& event) {
    // The bus no longer locks LVGL around dispatch, and SetActiveGroup renders.
    ScopedLvglLock lvgl_guard;

    try {
        auto action_it = event.payload.find(NavigationPayloadType::Action);
        if(action_it == event.payload.end())
            return;

        const auto* action_raw = std::get_if<uint32_t>(&action_it->second);
        if(action_raw == nullptr)
            return;

        if(static_cast<NavigationAction>(*action_raw) != NavigationAction::ShowGroup)
            return;

        auto group_it = event.payload.find(NavigationPayloadType::TargetGroupId);
        if(group_it == event.payload.end())
            return;

        const auto* group_id = std::get_if<uint32_t>(&group_it->second);
        if(group_id == nullptr)
            return;

        if(main_view_->SetActiveGroup(*group_id) != 0)
            LOG_ERR("Failed to show screen group %u.", *group_id);

        // The view is the source of truth; reconcile the service's optimistic state.
        if(auto active_group_id = main_view_->GetActiveGroupId())
            navigation_service_->SetActiveGroupId(*active_group_id);
    } catch(const std::exception& e) {
        LOG_ERR("Failed to apply navigation change. %s", e.what());
    } catch(...) {
        LOG_ERR("Failed to apply navigation change.");
    }
}

std::shared_ptr<NavigationService> UiController::GetNavigationService() const {
    return navigation_service_;
}

std::shared_ptr<SettingsRegistry> UiController::GetSettingsRegistry() const {
    return settings_registry_;
}

std::shared_ptr<IScreen> UiController::CreateScreen(std::shared_ptr<ScreenConfiguration> configuration) {
    try {
        return ScreenFactory::GetInstance().CreateScreen(
            configuration,
            main_view_->GetGroupContainer(configuration->group_id),
            widget_context_);
    } catch(const std::exception& e) {
        LOG_ERR("Failed to create screen with ID: %d. %s", configuration->id, e.what());
        return nullptr;
    }
}

void UiController::SetupTestConfiguration() {
    auto ui_configuration = make_shared_pmr<UiConfiguration>(Mrm::GetExtPmr());
    ui_configuration->active_screen_group_id = 0;

    auto screen_configuration = make_shared_pmr<ScreenConfiguration>(Mrm::GetExtPmr());
    screen_configuration->id = 0;
    screen_configuration->group_id = 0;
    screen_configuration->z_index = 0;
    screen_configuration->is_visible = true;
    screen_configuration->type = ScreenType::Gauge;

    // NOTE: Grid enables relative sizing, spliting screen in equal regions.
    // If width and height set to actual screen size positioning and size
    // will act as if they are set in px.
    // If set to smaller values actual screen size will be devided
    // in equal regions as a result positioning and sizing will expect cell indexes.
    screen_configuration->grid.snap_enabled = true;
    screen_configuration->grid.width = 466;
    screen_configuration->grid.height = 466;
    screen_configuration->grid.spacing_px = 0;

    // Widget 0: BasicIcon - Background
    auto widget0 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget0->type = WidgetType::BasicIcon;
    widget0->id = 0;
    widget0->position_grid.x = 0;
    widget0->position_grid.y = 0;
    widget0->size_grid.width = 466;
    widget0->size_grid.height = 466;
    widget0->z_index = 0;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE)] = static_cast<int>(IconType::Image);
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::FILE_PATH)] = "ui_img_norma_al88.bin";
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_WIDTH)] = 466;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_HEIGHT)] = 466;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = 0;
    screen_configuration->AddWidget(std::move(widget0));

    // Widget 1: IndicatorDigital
    auto widget1 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget1->type = WidgetType::IndicatorDigital;
    widget1->id = 1;
    widget1->position_grid.x = 0;
    widget1->position_grid.y = 230;
    widget1->size_grid.width = 200;
    widget1->size_grid.height = 100;
    widget1->z_index = 0;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_PRECISION)] = 2;
    // screen_configuration->AddWidget(std::move(widget1));

    // Widget 2: IndicatorHorizontalChart (Bar)
    auto widget2 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget2->type = WidgetType::IndicatorHorizontalChart;
    widget2->id = 2;
    widget2->position_grid.x = 0;
    widget2->position_grid.y = 0;
    widget2->size_grid.width = 466;
    widget2->size_grid.height = 160;
    widget2->z_index = 0;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = false;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT)] = 35;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE)] = static_cast<int>(HorizontalChartIndicatorType::Bar);
    screen_configuration->AddWidget(std::move(widget2));

    // Widget 3: IndicatorHorizontalChart (Line)
    auto widget3 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget3->type = WidgetType::IndicatorHorizontalChart;
    widget3->id = 3;
    widget3->position_grid.x = 0;
    widget3->position_grid.y = 280;
    widget3->size_grid.width = 466;
    widget3->size_grid.height = 200;
    widget3->z_index = 0;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE)] = static_cast<int>(HorizontalChartIndicatorType::Line);
    screen_configuration->AddWidget(std::move(widget3));

    auto widget4 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget4->type = WidgetType::IndicatorHorizontalChart;
    widget4->id = 4;
    widget4->position_grid.x = 0;
    widget4->position_grid.y = 0;
    widget4->size_grid.width = 466;
    widget4->size_grid.height = 160;
    widget4->z_index = 0;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // screen_configuration->AddWidget(std::move(widget4));

    // Widget: IndicatorArcFill
    auto widget5 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget5->type = WidgetType::IndicatorArcFill;
    widget5->id = 5;
    widget5->position_grid.x = 0;
    widget5->position_grid.y = 0;
    widget5->size_grid.width = 466;
    widget5->size_grid.height = 466;
    widget5->z_index = 0;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::START_ANGLE)] = 0;
    // widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::END_ANGLE)] = 360;
    // screen_configuration->AddWidget(std::move(widget5));

    // auto widget6 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    // widget6->type = WidgetType::IndicatorSegmentArc;
    // widget6->id = 6;
    // widget6->position_grid.x = 0;
    // widget6->position_grid.y = 0;
    // widget6->size_grid.width = 466;
    // widget6->size_grid.height = 466;
    // widget6->z_index = 0;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::START_ANGLE)] = 0;
    // // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::END_ANGLE)] = 360;
    // screen_configuration->AddWidget(std::move(widget6));

    // Widget: BasicArcIcon
    auto widget7 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget7->type = WidgetType::BasicArcIcon;
    widget7->id = 7;
    widget7->position_grid.x = 0;
    widget7->position_grid.y = 0;
    widget7->size_grid.width = 466;
    widget7->size_grid.height = 466;
    widget7->z_index = 0;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE)] = static_cast<int>(IconType::Dot);
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE)] = false;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = 0;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE)] = 180.0F;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET)] = 6;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::UI_SIGNAL_TYPE)] = static_cast<int>(UiSignalType::LoggingActive);
    screen_configuration->AddWidget(std::move(widget7));

    // auto widget8 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    // widget8->type = WidgetType::BasicArcIcon;
    // widget8->id = 8;
    // widget8->position_grid.x = 0;
    // widget8->position_grid.y = 0;
    // widget8->size_grid.width = 3;
    // widget8->size_grid.height = 3;
    // widget8->z_index = 0;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE)] = static_cast<int>(IconType::Label);
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE)] = false;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = 0;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE)] = -56.0F;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET)] = 2;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::LABEL)] = "log";
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::UI_SIGNAL_TYPE)] = static_cast<int>(UiSignalType::LoggingActive);
    // screen_configuration->AddWidget(std::move(widget8));

    // Widget: IndicatorDial
    auto widget9 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget9->type = WidgetType::IndicatorDial;
    widget9->id = 9;
    widget9->position_grid.x = 0;
    widget9->position_grid.y = 0;
    widget9->size_grid.width = 466;
    widget9->size_grid.height = 466;
    widget9->z_index = 0;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::FILE_PATH)] = "ui_img_arrow_al88.bin";
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_WIDTH)] = 15;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_HEIGHT)] = 220;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = -104;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::PIVOT_X)] = 7;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::PIVOT_Y)] = 7;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::START_ANGLE)] = 0;
    // widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::END_ANGLE)] = 360;
    screen_configuration->AddWidget(std::move(widget9));

    // Widget: IndicatorBar - Horizontal Left to right
    auto widget10 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget10->type = WidgetType::IndicatorBar;
    widget10->id = 10;
    widget10->position_grid.x = 0;
    widget10->position_grid.y = 180;
    widget10->size_grid.width = 466;
    widget10->size_grid.height = 8;
    widget10->z_index = 0;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    screen_configuration->AddWidget(std::move(widget10));

    // Widget: IndicatorBar - Vertical Bottom to top
    auto widget11 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget11->type = WidgetType::IndicatorBar;
    widget11->id = 11;
    widget11->position_grid.x = 200;
    widget11->position_grid.y = 0;
    widget11->size_grid.width = 40;
    widget11->size_grid.height = 466;
    widget11->z_index = 0;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::DIRECTION)] = static_cast<int>(InidicatorDirection::TopToBottom);
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // screen_configuration->AddWidget(std::move(widget11));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration));

    auto screen_configuration_1 = make_shared_pmr<ScreenConfiguration>(Mrm::GetExtPmr());
    screen_configuration_1->id = 1;
    screen_configuration_1->group_id = 1;
    screen_configuration_1->z_index = 0;
    screen_configuration_1->is_visible = true;
    screen_configuration_1->type = ScreenType::Gauge;
    screen_configuration_1->grid.snap_enabled = true;
    screen_configuration_1->grid.width = 466;
    screen_configuration_1->grid.height = 466;
    screen_configuration_1->grid.spacing_px = 0;

    // Widget 1_0: IndicatorDigital
    auto widget1_0 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget1_0->type = WidgetType::IndicatorDigital;
    widget1_0->id = 1;
    widget1_0->position_grid.x = 0;
    widget1_0->position_grid.y = 230;
    widget1_0->size_grid.width = 200;
    widget1_0->size_grid.height = 100;
    widget1_0->z_index = 0;
    widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_SOURCE)] = static_cast<int>(IndicatorValueSource::Sensor);
    // widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_PRECISION)] = 2;
    screen_configuration_1->AddWidget(std::move(widget1_0));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration_1));

    // A settings screen is pure configuration: no C++ is written per config screen.
    auto screen_configuration_2 = make_shared_pmr<ScreenConfiguration>(Mrm::GetExtPmr());
    screen_configuration_2->id = 2;
    screen_configuration_2->group_id = 2;
    screen_configuration_2->z_index = 0;
    screen_configuration_2->is_visible = true;
    screen_configuration_2->type = ScreenType::Settings;
    screen_configuration_2->grid.snap_enabled = true;
    screen_configuration_2->grid.width = 466;
    screen_configuration_2->grid.height = 466;
    screen_configuration_2->grid.spacing_px = 0;

    // Widget 2_0: IndicatorSetting - reads display.brightness back
    auto widget2_0 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget2_0->type = WidgetType::IndicatorSetting;
    widget2_0->id = 0;
    widget2_0->position_grid.x = 80;
    widget2_0->position_grid.y = 180;
    widget2_0->size_grid.width = 300;
    widget2_0->size_grid.height = 60;
    widget2_0->z_index = 0;
    widget2_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SETTING_ID)] = SettingId::DISPLAY_BRIGHTNESS;
    widget2_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::LABEL)] = "Brightness";
    screen_configuration_2->AddWidget(std::move(widget2_0));

    // Widget 2_1: ControlSlider - writes display.brightness
    auto widget2_1 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget2_1->type = WidgetType::ControlSlider;
    widget2_1->id = 1;
    widget2_1->position_grid.x = 80;
    widget2_1->position_grid.y = 140;
    widget2_1->size_grid.width = 300;
    widget2_1->size_grid.height = 60;
    widget2_1->z_index = 0;
    widget2_1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SETTING_ID)] = SettingId::DISPLAY_BRIGHTNESS;
    screen_configuration_2->AddWidget(std::move(widget2_1));

    // Widget 2_2: ControlButton - back to the gauge group
    auto widget2_2 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget2_2->type = WidgetType::ControlButton;
    widget2_2->id = 2;
    widget2_2->position_grid.x = 160;
    widget2_2->position_grid.y = 40;
    widget2_2->size_grid.width = 160;
    widget2_2->size_grid.height = 60;
    widget2_2->z_index = 0;
    widget2_2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::LABEL)] = "Back";
    widget2_2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::TARGET_GROUP)] = 0;
    screen_configuration_2->AddWidget(std::move(widget2_2));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration_2));

    ui_configuration_manager_->Update(*ui_configuration);
}

void UiController::SetupTestAssets() {
    if(ui_assets_manager_->Exists("ui_img_norma_al88.bin"))
        return;

    ui_assets_manager_->Save(
        "ui_img_norma_al88.bin",
        ui_img_norma_al88_data);

    ui_assets_manager_->Save(
        "ui_img_arrow_al88.bin",
        ui_img_arrow_al88_data);
}

} // namespace eerie_leap::controllers

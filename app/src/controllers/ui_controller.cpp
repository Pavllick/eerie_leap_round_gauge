#include <stdexcept>
#include <string>

#include <zephyr/logging/log.h>
#include <eerie_memory.hpp>

#include "utilities/memory/memory_resource_manager.h"

#include "configuration/cbor/cbor_ui_config/cbor_ui_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "domain/ui_domain/event_bus/ui_event_type.h"
#include "domain/ui_domain/models/widget_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/models/icon_type.h"
#include "domain/ui_domain/models/indicator_direction.h"

#include "views/screens/screen.h"
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

using eerie_leap::domain::ui_domain::event_bus::UiEventType;

LOG_MODULE_REGISTER(ui_controller_logger);

UiController::UiController(
    std::shared_ptr<IFsService> fs_service,
    std::shared_ptr<WorkQueueThread> config_work_queue_thread,
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame)
    : fs_service_(std::move(fs_service)),
      config_work_queue_thread_(std::move(config_work_queue_thread)),
      configuration_service_(std::move(configuration_service)),
      sensor_readings_frame_(std::move(sensor_readings_frame)) {}

int UiController::Initialize() {
    ThemeManager::GetInstance().SetTheme(make_shared_pmr<DarkBWTheme>(Mrm::GetExtPmr()));

    ui_renderer_service_ = make_shared_pmr<UiRendererService>(Mrm::GetExtPmr());
    if(ui_renderer_service_->Initialize() != 0) {
        LOG_ERR("Failed to initialize the UI renderer service.");
        return -1;
    }
    ui_renderer_service_->Start();

    auto cbor_ui_config_service = std::make_unique<config_services::CborConfigurationService<CborUiConfig>>(
        UI_CONFIGURATION_NAME, fs_service_, config_work_queue_thread_);
    ui_configuration_manager_ = make_shared_pmr<UiConfigurationManager>(
        Mrm::GetExtPmr(), std::move(cbor_ui_config_service));

    if(configuration_service_ != nullptr)
        configuration_service_->RegisterCborConfigurationManager(
            ConfigurationService::Type::Ui, ui_configuration_manager_);

    ui_assets_manager_ = std::make_shared<AssetsManager>(fs_service_, UI_ASSETS_DIR);

    // TODO: For test purposes only
    SetupTestConfiguration();
    SetupTestAssets();

    main_view_ = std::make_unique<MainView>();
    Configure(ui_configuration_manager_->Get());

    if(sensor_readings_frame_ != nullptr) {
        sensors_rendering_service_ = std::make_shared<SensorsRenderingService>(sensor_readings_frame_);
        sensors_rendering_service_->Initialize();
    }

    return 0;
}

int UiController::Start() {
    int res = main_view_->Render();
    if(res != 0)
        return res;

    if(sensors_rendering_service_ != nullptr)
        sensors_rendering_service_->Start();

    return 0;
}

int UiController::Configure(std::shared_ptr<UiConfiguration> config) {
    configuration_ = std::move(config);

    for(auto& screen_config : configuration_->screen_configurations) {
        auto screen = CreateScreen(screen_config);

        if(screen != nullptr) {
            screens_.emplace(screen_config->id, screen);
            main_view_->AddScreen(screen_config->id, screen);
        }
    }

    return 0;
}

std::shared_ptr<IScreen> UiController::CreateScreen(std::shared_ptr<ScreenConfiguration> configuration) {
    try {
        auto screen = std::make_shared<Screen>(ui_assets_manager_, configuration->id, main_view_->GetContainer());
        screen->Configure(configuration);

        return screen;
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
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE)] = static_cast<int>(UiEventType::LoggingStatusUpdated);
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
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE)] = static_cast<int>(UiEventType::LoggingStatusUpdated);
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
    // widget1_0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_PRECISION)] = 2;
    screen_configuration_1->AddWidget(std::move(widget1_0));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration_1));

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

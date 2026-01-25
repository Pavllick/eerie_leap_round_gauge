#include <memory>
#include <ctime>
#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "utilities/memory/memory_resource_manager.h"
#include "utilities/dev_tools/system_info.h"
#include "utilities/guid/guid_generator.h"

#include "subsys/device_tree/dt_configurator.h"
#include "subsys/device_tree/dt_fs.h"
#include "subsys/device_tree/dt_gpio.h"
#include "subsys/device_tree/dt_canbus.h"

#include "subsys/random/rng.h"
#include "subsys/fs/services/fs_service.h"
#include "subsys/gpio/gpio_buttons.h"
#include "subsys/time/time_service.h"
#include "subsys/time/rtc_provider.h"
#include "subsys/time/boot_elapsed_time_provider.h"
#include "subsys/event_bus/event_bus.h"

#include "configuration/services/cbor_configuration_service.h"

#include "domain/system_domain/configuration/system_configuration_manager.h"

#include "domain/canbus_domain/configuration/canbus_configuration_manager.h"
#include "domain/canbus_domain/services/canbus_service.h"

#include "domain/sensor_domain/sensor_readers/sensor_reader_factory.h"
#include "domain/sensor_domain/isr_sensor_readers/isr_sensor_reader_factory.h"
#include "domain/sensor_domain/services/sensors_processing_service.h"

#include "domain/ui_domain/configuration/ui_configuration_manager.h"
#include "domain/ui_domain/services/ui_renderer_service.h"
#include "domain/ui_domain/services/sensors_rendering_service.h"

#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/screen_configuration.h"
#include "domain/ui_domain/models/grid_settings.h"
#include "domain/ui_domain/models/widget_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_size.h"
#include "domain/ui_domain/models/widget_position.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/models/icon_type.h"
#include "domain/ui_domain/assets_manager/ui_assets_manager.h"
#include "domain/canbus_com_domain/services/canbus_com_service.h"

#include "controllers/ui_controller.h"
#include "controllers/logging_controller.h"

#include "views/utilitites/enums.h"
#include "views/themes/theme_manager.h"
#include "views/themes/dark_theme.h"
#include "views/themes/dark_bw_theme.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"
#include "views/assets/images/images_register.h"

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
using namespace eerie_leap::utilities::guid;

using namespace eerie_leap::subsys::random;
using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::subsys::gpio;
using namespace eerie_leap::subsys::time;

using namespace eerie_leap::configuration::services;

using namespace eerie_leap::domain::system_domain::configuration;
using namespace eerie_leap::domain::ui_domain;
using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::ui_domain::services;
using namespace eerie_leap::domain::ui_domain::assets_manager;

using namespace eerie_leap::domain::canbus_domain::configuration;
using namespace eerie_leap::domain::canbus_domain::services;
using namespace eerie_leap::domain::canbus_domain::models;
using namespace eerie_leap::domain::canbus_com_domain::services;
using namespace eerie_leap::domain::sensor_domain::services;
using namespace eerie_leap::domain::sensor_domain::sensor_readers;
using namespace eerie_leap::domain::sensor_domain::isr_sensor_readers;

using namespace eerie_leap::controllers;

using namespace eerie_leap::views::themes;
using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::indicators;
using namespace eerie_leap::views::assets::images;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 5000;

void SetupCanbusConfiguration(std::shared_ptr<CanbusConfigurationManager> canbus_configuration_manager);
void SetupTestSensors(std::shared_ptr<SensorsConfigurationManager> sensors_configuration_manager);
std::shared_ptr<UiConfiguration> SetupTestUiConfig(std::shared_ptr<UiConfigurationManager> ui_configuration_manager);
void SetupTestUiAssets(std::shared_ptr<UiAssetsManager> ui_assets_manager);
void EmulateReadings(
    std::shared_ptr<GuidGenerator> guid_generator,
    std::shared_ptr<SensorsConfigurationManager> sensors_configuration_manager,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame);

int main() {
    DtConfigurator::Initialize();

    auto dark_theme = make_shared_ext<DarkBWTheme>();
    ThemeManager::GetInstance().SetTheme(dark_theme);

    auto ui_renderer_service = make_shared_ext<UiRendererService>();
    if(ui_renderer_service->Initialize() != 0)
        return -1;
    ui_renderer_service->Start();

    auto fs_service = make_shared_ext<FsService>(DtFs::GetInternalFsMp().value());
    if(!fs_service->Initialize()) {
        LOG_ERR("Failed to initialize File System.");
        return -1;
    }

    auto rtc_provider = std::make_shared<RtcProvider>();
    auto boot_elapsed_time_provider = std::make_shared<BootElapsedTimeProvider>();
    auto time_service = std::make_shared<TimeService>(rtc_provider, boot_elapsed_time_provider);
    time_service->Initialize();

    auto guid_generator = make_shared_ext<GuidGenerator>();

    auto system_config_service = std::make_unique<CborConfigurationService<CborSystemConfig>>(
        "system_config", fs_service);
    auto system_configuration_manager = make_shared_ext<SystemConfigurationManager>(
        std::move(system_config_service));

    auto cbor_canbus_config_service = std::make_unique<CborConfigurationService<CborCanbusConfig>>(
        "canbus_config", fs_service);
    auto json_canbus_config_service = std::make_unique<JsonConfigurationService<JsonCanbusConfig>>(
        "canbus_config", nullptr);
    auto canbus_configuration_manager = std::make_shared<CanbusConfigurationManager>(
        std::move(cbor_canbus_config_service), std::move(json_canbus_config_service), nullptr);

    auto cbor_sensors_config_service = std::make_unique<CborConfigurationService<CborSensorsConfig>>(
        "sensors_config", fs_service);
    auto json_sensors_config_service = std::make_unique<JsonConfigurationService<JsonSensorsConfig>>(
        "sensors_config", nullptr);
    auto sensors_configuration_manager = std::make_shared<SensorsConfigurationManager>(
        std::move(cbor_sensors_config_service),
        std::move(json_sensors_config_service),
        nullptr,
        0, 0);

    auto ui_config_service = std::make_unique<CborConfigurationService<CborUiConfig>>(
        "ui_config", fs_service);
    auto ui_configuration_manager = make_shared_ext<UiConfigurationManager>(
        std::move(ui_config_service));

    // TODO: For test purposes only
    SetupTestUiConfig(ui_configuration_manager);

    auto ui_assets_manager = std::make_shared<UiAssetsManager>(fs_service);

    // TODO: For test purposes only
    SetupTestUiAssets(ui_assets_manager);

    auto ui_controller = make_shared_ext<UiController>(ui_configuration_manager, ui_assets_manager);
    ui_controller->Render();

    int input_work_queue_stack_size = 4096;
    int input_work_queue_priority = 5;
    auto input_work_queue_thread = std::make_shared<WorkQueueThread>(
        "input_work_queue",
        input_work_queue_stack_size,
        input_work_queue_priority);
    input_work_queue_thread->Initialize();

    // TODO: For test purposes only
    SetupCanbusConfiguration(canbus_configuration_manager);

    auto canbus_service = std::make_shared<CanbusService>(
        DtCanbus::Get, canbus_configuration_manager);

    auto canbus_com_service = std::make_shared<CanbusComService>(canbus_service);
    canbus_com_service->Initialize();
    canbus_com_service->Start();

    auto sensor_readings_frame = make_shared_pmr<SensorReadingsFrame>(Mrm::GetExtPmr());

    auto isr_sensor_reader_factory = std::make_shared<IsrSensorReaderFactory>(
        time_service,
        guid_generator,
        sensor_readings_frame,
        canbus_service);

    auto sensors_processing_service = std::make_shared<SensorsProcessingService>(
        sensors_configuration_manager,
        sensor_readings_frame,
        isr_sensor_reader_factory,
        nullptr);
    sensors_processing_service->Initialize();

    // TODO: For test purposes only
    SetupTestSensors(sensors_configuration_manager);

    auto sensors_rendering_service = std::make_shared<SensorsRenderingService>(
        time_service,
        canbus_configuration_manager,
        canbus_service,
        sensor_readings_frame);
    sensors_rendering_service->Initialize();
    sensors_rendering_service->Start();

    std::shared_ptr<LoggingController> logging_controller;
    do {
        if(canbus_com_service == nullptr) {
            LOG_WRN("CANBus COM service not configured.");
            break;
        }

        if(!DtGpio::GetButtons().has_value()) {
            LOG_WRN("No buttons configured.");
            break;
        }

        auto gpio_buttons = make_shared_ext<GpioButtons>(DtGpio::GetButtons().value());
        if(gpio_buttons->Initialize() != 0) {
            LOG_ERR("Failed to initialize buttons.");
            break;
        }

        logging_controller = make_shared_ext<LoggingController>(
            gpio_buttons, input_work_queue_thread, canbus_com_service);
        logging_controller->Initialize();
    } while(false);

    sensors_processing_service->Start();

	while (true) {
        // SystemInfo::PrintHeapInfo();
        // SystemInfo::PrintStackInfo();
        k_msleep(SLEEP_TIME_MS);

        // TODO: For test purposes only
        // EmulateReadings(guid_generator, sensors_configuration_manager, sensor_readings_frame);
        // k_msleep(20);
	}

	return 0;
}

void EmulateReadings(
    std::shared_ptr<GuidGenerator> guid_generator,
    std::shared_ptr<SensorsConfigurationManager> sensors_configuration_manager,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame) {
    for(auto sensor : *sensors_configuration_manager->Get()) {
        SensorReading reading(std::allocator_arg, Mrm::GetDefaultPmr(), guid_generator->Generate(), sensor);
        reading.source = ReadingSource::PROCESSING;
        reading.status = ReadingStatus::PROCESSED;
        reading.value = (Rng::Get32() / static_cast<float>(UINT32_MAX)) * 100.0F;

        sensor_readings_frame->AddOrUpdateReading(reading);
    }
}

void SetupCanbusConfiguration(std::shared_ptr<CanbusConfigurationManager> canbus_configuration_manager) {
    auto canbus_configuration = make_shared_pmr<CanbusConfiguration>(Mrm::GetExtPmr());
    canbus_configuration->com_bus_channel = 0;

    CanChannelConfiguration canbus_channel_configuration_0(std::allocator_arg, Mrm::GetExtPmr());
    canbus_channel_configuration_0.type = CanbusType::CLASSICAL_CAN;
    canbus_channel_configuration_0.is_extended_id = false;
    canbus_channel_configuration_0.bus_channel = 0;
    canbus_channel_configuration_0.bitrate = 1000000;
    // canbus_channel_configuration_0.data_bitrate = 2000000;

    // auto message_configuration_0 = make_shared_pmr<CanMessageConfiguration>(Mrm::GetExtPmr());
    // message_configuration_0->name = "EL_FRAME_0";
    // message_configuration_0->message_size = 8;
    // message_configuration_0->frame_id = 790;

    // CanSignalConfiguration signal_configuration_0(std::allocator_arg, Mrm::GetExtPmr());
    // signal_configuration_0.start_bit = 16;
    // signal_configuration_0.size_bits = 16;
    // signal_configuration_0.name = "RPM";
    // signal_configuration_0.unit = "rpm";
    // signal_configuration_0.factor = 0.1;
    // message_configuration_0->signal_configurations.emplace_back(std::move(signal_configuration_0));
    // canbus_channel_configuration_0.message_configurations.emplace_back(std::move(message_configuration_0));

    for(int i = 0; i < 10; i++) {
        auto message_configuration = make_shared_pmr<CanMessageConfiguration>(Mrm::GetExtPmr());
        message_configuration->frame_id = 100 + i;
        message_configuration->name = "EL_FRAME_" + std::to_string(i);
        message_configuration->message_size = 8;

        CanSignalConfiguration signal_configuration(std::allocator_arg, Mrm::GetExtPmr());
        signal_configuration.start_bit = 0;
        signal_configuration.size_bits = 16;
        signal_configuration.name = "sensor_" + std::to_string(i);
        signal_configuration.unit = "km/h";
        message_configuration->signal_configurations.emplace_back(std::move(signal_configuration));

        canbus_channel_configuration_0.message_configurations.emplace_back(std::move(message_configuration));
    }

    canbus_configuration->channel_configurations.emplace(
        canbus_channel_configuration_0.bus_channel,
        std::move(canbus_channel_configuration_0));

    canbus_configuration_manager->Update(*canbus_configuration);
}

void SetupTestSensors(std::shared_ptr<SensorsConfigurationManager> sensors_configuration_manager) {
    // Test Sensors

    // auto sensor_1 = make_shared_pmr<Sensor>(Mrm::GetExtPmr(), "sensor_1");

    // sensor_1->metadata.name = "Sensor 1";
    // sensor_1->metadata.unit = "";
    // sensor_1->metadata.description = "Test Sensor 1";

    // sensor_1->configuration.type = SensorType::CANBUS_ANALOG;
    // sensor_1->configuration.canbus_source = make_unique_pmr<CanbusSource>(Mrm::GetExtPmr(), 0, 790, "RPM");

    // std::vector<std::shared_ptr<Sensor>> sensors = {
    //     sensor_1
    // };

    std::vector<std::shared_ptr<Sensor>> sensors;

    for(int i = 0; i < 10; i++) {
        auto sensor = make_shared_pmr<Sensor>(Mrm::GetExtPmr(), "sensor_" + std::to_string(i));

        sensor->metadata.name = "Sensor 1";
        sensor->metadata.unit = "";
        sensor->metadata.description = "Test Sensor 1";

        sensor->configuration.type = SensorType::CANBUS_ANALOG;
        sensor->configuration.canbus_source = make_unique_pmr<CanbusSource>(Mrm::GetExtPmr(), 0, 100 + i, "sensor_" + std::to_string(i));

        sensors.push_back(sensor);
    }

    sensors_configuration_manager->Update(sensors);
}

std::shared_ptr<UiConfiguration> SetupTestUiConfig(std::shared_ptr<UiConfigurationManager> ui_configuration_manager) {
    auto ui_configuration = make_shared_pmr<UiConfiguration>(Mrm::GetExtPmr());
    ui_configuration->active_screen_index = 0;

    auto screen_configuration = make_shared_pmr<ScreenConfiguration>(Mrm::GetExtPmr());
    screen_configuration->id = 0;
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
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE)] = static_cast<int>(IconType::Image);
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::FILE_PATH)] = "ui_img_norma_al88.bin";
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_WIDTH)] = 466;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_HEIGHT)] = 466;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    widget0->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = 0;
    screen_configuration->widget_configurations.push_back(std::move(widget0));

    // Widget 1: IndicatorDigital
    auto widget1 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget1->type = WidgetType::IndicatorDigital;
    widget1->id = 1;
    widget1->position_grid.x = 0;
    widget1->position_grid.y = 230;
    widget1->size_grid.width = 200;
    widget1->size_grid.height = 100;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    // widget1.properties[WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_PRECISION)] = 2;
    screen_configuration->widget_configurations.push_back(std::move(widget1));

    // Widget 2: IndicatorHorizontalChart (Bar)
    auto widget2 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget2->type = WidgetType::IndicatorHorizontalChart;
    widget2->id = 2;
    widget2->position_grid.x = 0;
    widget2->position_grid.y = 0;
    widget2->size_grid.width = 466;
    widget2->size_grid.height = 160;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = false;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT)] = 35;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE)] = static_cast<int>(HorizontalChartIndicatorType::Bar);
    screen_configuration->widget_configurations.push_back(std::move(widget2));

    // Widget 3: IndicatorHorizontalChart (Line)
    auto widget3 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget3->type = WidgetType::IndicatorHorizontalChart;
    widget3->id = 3;
    widget3->position_grid.x = 0;
    widget3->position_grid.y = 280;
    widget3->size_grid.width = 466;
    widget3->size_grid.height = 200;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE)] = static_cast<int>(HorizontalChartIndicatorType::Line);
    screen_configuration->widget_configurations.push_back(std::move(widget3));

    auto widget4 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget4->type = WidgetType::IndicatorHorizontalChart;
    widget4->id = 4;
    widget4->position_grid.x = 0;
    widget4->position_grid.y = 0;
    widget4->size_grid.width = 466;
    widget4->size_grid.height = 160;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget4->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    // screen_configuration->widget_configurations.push_back(std::move(widget4));

    // Widget: IndicatorArcFill
    auto widget5 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget5->type = WidgetType::IndicatorArcFill;
    widget5->id = 5;
    widget5->position_grid.x = 0;
    widget5->position_grid.y = 0;
    widget5->size_grid.width = 466;
    widget5->size_grid.height = 466;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    // widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::START_ANGLE)] = 0;
    // widget5->properties[WidgetProperty::GetTypeName(WidgetPropertyType::END_ANGLE)] = 360;
    // screen_configuration->widget_configurations.push_back(std::move(widget5));

    // auto widget6 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    // widget6->type = WidgetType::IndicatorSegmentArc;
    // widget6->id = 6;
    // widget6->position_grid.x = 0;
    // widget6->position_grid.y = 0;
    // widget6->size_grid.width = 466;
    // widget6->size_grid.height = 466;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    // // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::START_ANGLE)] = 0;
    // // widget6->properties[WidgetProperty::GetTypeName(WidgetPropertyType::END_ANGLE)] = 360;
    // screen_configuration->widget_configurations.push_back(std::move(widget6));

    // Widget: BasicArcIcon
    auto widget7 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget7->type = WidgetType::BasicArcIcon;
    widget7->id = 7;
    widget7->position_grid.x = 0;
    widget7->position_grid.y = 0;
    widget7->size_grid.width = 466;
    widget7->size_grid.height = 466;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE)] = static_cast<int>(IconType::Dot);
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE)] = false;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = 0;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE)] = 180.0F;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET)] = 6;
    widget7->properties[WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE)] = static_cast<int>(UiEventType::LoggingStatusUpdated);
    screen_configuration->widget_configurations.push_back(std::move(widget7));

    // auto widget8 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    // widget8->type = WidgetType::BasicArcIcon;
    // widget8->id = 8;
    // widget8->position_grid.x = 0;
    // widget8->position_grid.y = 0;
    // widget8->size_grid.width = 3;
    // widget8->size_grid.height = 3;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE)] = static_cast<int>(IconType::Label);
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE)] = false;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X)] = 0;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y)] = 0;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE)] = -56.0F;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET)] = 2;
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::LABEL)] = "log";
    // widget8->properties[WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE)] = static_cast<int>(UiEventType::LoggingStatusUpdated);
    // screen_configuration->widget_configurations.push_back(std::move(widget8));

    // Widget: IndicatorDial
    auto widget9 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget9->type = WidgetType::IndicatorDial;
    widget9->id = 9;
    widget9->position_grid.x = 0;
    widget9->position_grid.y = 0;
    widget9->size_grid.width = 466;
    widget9->size_grid.height = 466;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::FILE_PATH)] = "ui_img_arrow_al88.bin";
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_WIDTH)] = 15;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IMG_HEIGHT)] = 220;
    widget9->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
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
    screen_configuration->widget_configurations.push_back(std::move(widget9));

    // Widget: IndicatorBar - Horizontal Left to right
    auto widget10 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget10->type = WidgetType::IndicatorBar;
    widget10->id = 10;
    widget10->position_grid.x = 0;
    widget10->position_grid.y = 180;
    widget10->size_grid.width = 466;
    widget10->size_grid.height = 40;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget10->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    screen_configuration->widget_configurations.push_back(std::move(widget10));

    // Widget: IndicatorBar - Vertical Bottom to top
    auto widget11 = make_shared_pmr<WidgetConfiguration>(Mrm::GetExtPmr());
    widget11->type = WidgetType::IndicatorBar;
    widget11->id = 11;
    widget11->position_grid.x = 200;
    widget11->position_grid.y = 0;
    widget11->size_grid.width = 40;
    widget11->size_grid.height = 466;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::DIRECTION)] = static_cast<int>(InidicatorDirection::TopToBottom);
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE)] = true;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget11->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    // screen_configuration->widget_configurations.push_back(std::move(widget11));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration));

    ui_configuration_manager->Update(*ui_configuration);

    return ui_configuration;
}

void SetupTestUiAssets(std::shared_ptr<UiAssetsManager> ui_assets_manager) {
    ui_assets_manager->Save(
        "ui_img_norma_al88.bin",
        ui_img_norma_al88_data);

    ui_assets_manager->Save(
        "ui_img_arrow_al88.bin",
        ui_img_arrow_al88_data);
}

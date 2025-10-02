#include <memory>
#include <ctime>
#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "utilities/memory/heap_allocator.h"
#include "utilities/dev_tools/system_info.h"
#include "utilities/guid/guid_generator.h"

#include "subsys/device_tree/dt_configurator.h"
#include "subsys/device_tree/dt_fs.h"
#include "subsys/device_tree/dt_modbus.h"
#include "subsys/device_tree/dt_gpio.h"

#include "subsys/random/rng.h"
#include "subsys/fs/services/fs_service.h"
#include "subsys/modbus/modbus.h"
#include "subsys/gpio/gpio_buttons.h"

#include "domain/interface_domain/interface.h"
#include "domain/ui_domain/ui_renderer.h"
#include "domain/sensor_domain/models/sensor.h"
#include "domain/interface_domain/processors/reading_processor.h"
#include "domain/interface_domain/processors/status_processor.h"

#include "controllers/ui_controller.h"
#include "controllers/logging_controller.h"

#include "configuration/services/configuration_service.h"
#include "configuration/system_config/system_config.h"

#include "domain/system_domain/configuration/system_configuration_manager.h"
#include "domain/ui_domain/configuration/ui_configuration_manager.h"

#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/screen_configuration.h"
#include "domain/ui_domain/models/grid_settings.h"
#include "domain/ui_domain/models/widget_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_size.h"
#include "domain/ui_domain/models/widget_position.h"
#include "domain/ui_domain/models/widget_property.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

#include "subsys/event_bus/event_bus.h"
#include "views/themes/theme_manager.h"
#include "views/themes/dark_theme.h"

using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::indicators;

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
using namespace eerie_leap::utilities::guid;

using namespace eerie_leap::controllers;

using namespace eerie_leap::subsys::random;
using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::subsys::modbus;
using namespace eerie_leap::subsys::gpio;

using namespace eerie_leap::domain::system_domain::configuration;
using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::interface_domain::processors;
using namespace eerie_leap::domain::ui_domain;
using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::sensor_domain::models;
using namespace eerie_leap::configuration::services;

using namespace eerie_leap::controllers;
using namespace eerie_leap::views::themes;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 200;

std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> SetupTestSensors();
std::shared_ptr<UiConfiguration> SetupTestUiConfig(std::shared_ptr<UiConfigurationManager> ui_configuration_manager);

int main() {
    DtConfigurator::Initialize();

    auto dark_theme = make_shared_ext<DarkTheme>();
    ThemeManager::GetInstance().SetTheme(dark_theme);

    auto ui_renderer = make_shared_ext<UiRenderer>();
    if(ui_renderer->Initialize() != 0)
        return -1;

    ui_renderer->Start();

    auto fs_service = make_shared_ext<FsService>(DtFs::GetInternalFsMp().value());
    if(!fs_service->Initialize()) {
        LOG_ERR("Failed to initialize File System.");
        return -1;
    }

    auto guid_generator = make_shared_ext<GuidGenerator>();

    auto reading_processor = make_shared_ext<ReadingProcessor>();
    auto status_processor = make_shared_ext<StatusProcessor>();

    auto system_config_service = make_shared_ext<ConfigurationService<SystemConfig>>("system_config", fs_service);
    auto system_configuration_manager = make_shared_ext<SystemConfigurationManager>(system_config_service);

    auto ui_config_service = make_shared_ext<ConfigurationService<UiConfig>>("ui_config", fs_service);
    auto ui_configuration_manager = make_shared_ext<UiConfigurationManager>(ui_config_service);

    std::shared_ptr<Interface> interface = nullptr;
    if(DtModbus::Get().has_value()) {
        auto modbus = make_shared_ext<Modbus>(
            DtModbus::Get().value(),
            system_configuration_manager->Get()->interface_channel);

        interface = make_shared_ext<Interface>(modbus, system_configuration_manager, reading_processor);
        if(interface->Initialize() != 0)
            return -1;

        interface->RegisterProcessor<SensorReadingDto>(reading_processor);
        interface->RegisterProcessor<UserStatus>(status_processor);
    }

    // NOTE: This is a temporary solution.
    // auto sensors = SetupTestSensors();
    SetupTestUiConfig(ui_configuration_manager);

    auto ui_controller = make_shared_ext<UiController>(ui_configuration_manager);
    ui_controller->Render();

    std::shared_ptr<LoggingController> logging_controller;
    do {
        if(interface == nullptr) {
            LOG_WRN("No interface configured.");
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

        logging_controller = make_shared_ext<LoggingController>(gpio_buttons, interface);
        logging_controller->Initialize();
    } while(false);

#ifdef CONFIG_BOARD_NATIVE_SIM
    SensorReadingDto reading;
    reading.id = guid_generator->Generate();
    reading.sensor_id_hash = 2348664336;
    reading.timestamp_ms = std::chrono::milliseconds(static_cast<int64_t>(k_uptime_get()));
    reading.value = 0;
    reading.status = ReadingStatus::PROCESSED;
#endif // CONFIG_BOARD_NATIVE_SIM

	while (true) {
    #ifdef CONFIG_BOARD_NATIVE_SIM
        reading.value = (Rng::Get32() / static_cast<float>(UINT32_MAX)) * 100;
        reading_processor->Process(reading);
    #endif // CONFIG_BOARD_NATIVE_SIM

        k_msleep(SLEEP_TIME_MS);

        // SystemInfo::print_heap_info();
        // SystemInfo::print_stack_info();
	}

	return 0;
}

std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> SetupTestSensors() {
    Sensor sensor_1 {
        .id = "sensor_1",
        .id_hash = 2348664336,
        .metadata = {
            .name = "Sensor 1",
            .unit = "km/h",
            .description = "Test Sensor 1"
        },
        .configuration = {
            .type = SensorType::PHYSICAL_ANALOG,
            .channel = 0,
            .sampling_rate_ms = 10
        }
    };

    Sensor sensor_2 {
        .id = "sensor_2",
        .metadata = {
            .name = "Sensor 2",
            .unit = "km/h",
            .description = "Test Sensor 2"
        },
        .configuration = {
            .type = SensorType::PHYSICAL_ANALOG,
            .channel = 1,
            .sampling_rate_ms = 500
        }
    };

    Sensor sensor_3 {
        .id = "sensor_3",
        .metadata = {
            .name = "Sensor 3",
            .unit = "km/h",
            .description = "Test Sensor 3"
        },
        .configuration = {
            .type = SensorType::VIRTUAL_ANALOG,
            .sampling_rate_ms = 2000
        }
    };

    Sensor sensor_4 {
        .id = "sensor_4",
        .metadata = {
            .name = "Sensor 4",
            .unit = "",
            .description = "Test Sensor 4"
        },
        .configuration = {
            .type = SensorType::PHYSICAL_INDICATOR,
            .channel = 1,
            .sampling_rate_ms = 1000
        }
    };

    Sensor sensor_5 {
        .id = "sensor_5",
        .metadata = {
            .name = "Sensor 5",
            .unit = "",
            .description = "Test Sensor 5"
        },
        .configuration = {
            .type = SensorType::VIRTUAL_INDICATOR,
            .sampling_rate_ms = 1000
        }
    };

    std::vector<std::shared_ptr<Sensor>> sensors = {
        make_shared_ext<Sensor>(sensor_1),
        make_shared_ext<Sensor>(sensor_2),
        make_shared_ext<Sensor>(sensor_3),
        // make_shared_ext<Sensor>(sensor_4),
        // make_shared_ext<Sensor>(sensor_5)
    };

    auto sensors_ptr = make_shared_ext<std::vector<std::shared_ptr<Sensor>>>(sensors);

    return sensors_ptr;
}

std::shared_ptr<UiConfiguration> SetupTestUiConfig(std::shared_ptr<UiConfigurationManager> ui_configuration_manager) {
    auto ui_configuration = make_shared_ext<UiConfiguration>();
    ui_configuration->active_screen_index = 0;

    ScreenConfiguration screen_configuration {
        .id = 0,
        .type = ScreenType::Gauge,
        .grid = GridSettings {
            .snap_enabled = true,
            .width = 3,
            .height = 3,
            .spacing_px = 0
        },
        .widget_configurations = {
            WidgetConfiguration {
                .type = WidgetType::IndicatorDigital,
                .id = 1,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 1
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 1
                },
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                    // { WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_PRECISION), 2 }
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorHorizontalChart,
                .id = 2,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 0
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 1
                },
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED), false },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT), 35 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE), static_cast<int>(HorizontalChartIndicatorType::BAR) },
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorHorizontalChart,
                .id = 3,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 1
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 1
                },
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE), static_cast<int>(HorizontalChartIndicatorType::LINE) }
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorHorizontalChart,
                .id = 4,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 2
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 1
                },
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorArcFill,
                .id = 0,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 0
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 3
                },
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" }
                }
            },
            WidgetConfiguration {
                .type = WidgetType::BasicArcLabelIcon,
                .id = 5,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 0
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 3
                },
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE), true },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE), false },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE), -56.0F },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET), 2 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::LABEL), "log" },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE), static_cast<int>(UiEventType::LOGGING_STATUS_UPDATED) }
                }
            }
        }
    };

    ui_configuration->screen_configurations.push_back(screen_configuration);

    ui_configuration_manager->Update(ui_configuration);

    return ui_configuration;
}

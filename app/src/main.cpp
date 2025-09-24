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

#include "subsys/fs/services/fs_service.h"
#include "subsys/modbus/modbus.h"
#include "subsys/gpio/gpio_buttons.h"

#include "domain/interface_domain/interface.h"
#include "domain/ui_domain/ui_renderer.h"
#include "domain/sensor_domain/models/sensor.h"
#include "domain/sensor_domain/services/reading_processor_service.h"

#include "controllers/gague_controller.h"

#include "configuration/services/configuration_service.h"
#include "configuration/system_config/system_config.h"

#include "controllers/configuration/system_configuration_controller.h"
#include "controllers/configuration/gauge_configuration_controller.h"

#include "views/configuration/gauge_configuration.h"
#include "views/screens/configuration/screen_configuration.h"
#include "views/screens/configuration/grid_settings.h"
#include "views/widgets/configuration/widget_configuration.h"
#include "views/widgets/configuration/widget_type.h"
#include "views/widgets/configuration/widget_size.h"
#include "views/widgets/configuration/widget_position.h"
#include "views/widgets/configuration/widget_property.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::indicators;

using namespace eerie_leap::views::configuration;
using namespace eerie_leap::views::screens::configuration;
using namespace eerie_leap::views::widgets::configuration;

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
using namespace eerie_leap::utilities::guid;

using namespace eerie_leap::controllers;

using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::subsys::modbus;
using namespace eerie_leap::subsys::gpio;

using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::ui_domain;
using namespace eerie_leap::domain::sensor_domain::models;
using namespace eerie_leap::domain::sensor_domain::services;
using namespace eerie_leap::configuration::services;

using namespace eerie_leap::controllers;
using namespace eerie_leap::controllers::configuration;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 2000;

std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> SetupTestSensors();
std::shared_ptr<GaugeConfiguration> SetupTestGaugeConfig();

int main() {
    DtConfigurator::Initialize();

    auto fs_service = make_shared_ext<FsService>(DtFs::GetInternalFsMp().value());
    if(!fs_service->Initialize()) {
        LOG_ERR("Failed to initialize File System.");
        return -1;
    }

    auto guid_generator = make_shared_ext<GuidGenerator>();

    auto reading_processor_service = make_shared_ext<ReadingProcessorService>();
    reading_processor_service->Initialize();

    auto system_config_service = make_shared_ext<ConfigurationService<SystemConfig>>("system_config", fs_service);
    auto system_configuration_controller = make_shared_ext<SystemConfigurationController>(system_config_service);

    auto gauge_config_service = make_shared_ext<ConfigurationService<GaugeConfig>>("gauge_config", fs_service);
    auto gauge_configuration_controller = make_shared_ext<GaugeConfigurationController>(gauge_config_service);

    std::shared_ptr<Interface> interface = nullptr;
    if(DtModbus::Get().has_value()) {
    auto modbus = make_shared_ext<Modbus>(
            DtModbus::Get().value(),
        system_configuration_controller->Get()->interface_channel);

        interface = make_shared_ext<Interface>(modbus, system_configuration_controller, reading_processor_service);
    if(interface->Initialize() != 0)
        return -1;
    }

    // NOTE: This is a temporary solution.
    auto sensors = SetupTestSensors();
    auto gauge_config = SetupTestGaugeConfig();
    gauge_configuration_controller->Update(gauge_config);

    auto widget_factory = WidgetFactory::GetInstance();

    auto gague_controller = make_shared_ext<GagueController>(
        gauge_configuration_controller,
        sensors,
        reading_processor_service,
        widget_factory);
    gague_controller->Render();

    auto ui_renderer = make_shared_ext<UiRenderer>();
    if(ui_renderer->Initialize() != 0)
        return -1;

    ui_renderer->Start();

	while (true) {
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

std::shared_ptr<GaugeConfiguration> SetupTestGaugeConfig() {
    auto gauge_configuration = make_shared_ext<GaugeConfiguration>();
    gauge_configuration->active_screen_index = 0;

    ScreenConfiguration screen_configuration {
        .id = 0,
        .grid = GridSettings {
            .snap_enabled = true,
            .width = 3,
            .height = 3,
            .spacing_px = 0
        },
        .widget_configurations = {
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
                .is_animation_enabled = true,
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" }
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorDigital,
                .id = 1,
                .position_grid = WidgetPosition {
                    .x = 1,
                    .y = 1
                },
                .size_grid = WidgetSize {
                    .width = 1,
                    .height = 1
                },
                .is_animation_enabled = true,
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" }
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
                .is_animation_enabled = false,
            .properties = {
            { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
            { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT), 35 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE), static_cast<std::uint16_t>(HorizontalChartIndicatorType::LINE) }
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorHorizontalChart,
                .id = 2,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 1
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 1
                },
                .is_animation_enabled = true,
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE), static_cast<std::uint16_t>(HorizontalChartIndicatorType::LINE) }
                }
            },
            WidgetConfiguration {
                .type = WidgetType::IndicatorHorizontalChart,
                .id = 2,
                .position_grid = WidgetPosition {
                    .x = 0,
                    .y = 2
                },
                .size_grid = WidgetSize {
                    .width = 3,
                    .height = 1
                },
                .is_animation_enabled = true,
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
            }
}
        }
    };

    gauge_configuration->screen_configurations.push_back(screen_configuration);

    return gauge_configuration;
}

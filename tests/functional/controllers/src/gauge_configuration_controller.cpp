#include <memory>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "configuration/gauge_config/gauge_config.h"
#include "configuration/services/configuration_service.h"
#include "controllers/configuration/gauge_configuration_controller.h"

#include "subsys/device_tree/dt_fs.h"
#include "subsys/fs/services/fs_service.h"
#include "views/widgets/configuration/widget_property.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::controllers::configuration;
using namespace eerie_leap::views::widgets::configuration;
using namespace eerie_leap::views::widgets::indicators;

ZTEST_SUITE(gauge_configuration_controller, NULL, NULL, NULL, NULL, NULL);

std::shared_ptr<GaugeConfiguration> gauge_configuration_controller_test_SetupTestGaugeConfig() {
    auto gauge_configuration = make_shared_ext<GaugeConfiguration>();
    gauge_configuration->active_screen_index = 8;

    ScreenConfiguration screen_configuration {
        .id = 8,
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
            }
        }
    };

    gauge_configuration->screen_configurations.push_back(screen_configuration);

    return gauge_configuration;
}

ZTEST(gauge_configuration_controller, test_GaugeConfigurationController_Save_config_successfully_saved) {
    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();

    auto gauge_configuration_service = std::make_shared<ConfigurationService<GaugeConfig>>("gauge_config", fs_service);
    auto gauge_configuration_controller = std::make_shared<GaugeConfigurationController>(gauge_configuration_service);

    auto gauge_config = gauge_configuration_controller_test_SetupTestGaugeConfig();
    bool result = gauge_configuration_controller->Update(gauge_config);
    zassert_true(result);
}

ZTEST(gauge_configuration_controller, test_GaugeConfigurationController_Save_config_and_Load) {
    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();

    auto gauge_configuration_service = std::make_shared<ConfigurationService<GaugeConfig>>("gauge_config", fs_service);
    auto gauge_configuration_controller = std::make_shared<GaugeConfigurationController>(gauge_configuration_service);

    auto gauge_config = gauge_configuration_controller_test_SetupTestGaugeConfig();
    bool result = gauge_configuration_controller->Update(gauge_config);
    zassert_true(result);

    auto saved_gauge_configuration = *gauge_configuration_controller->Get(true);

    zassert_equal(saved_gauge_configuration.active_screen_index, gauge_config->active_screen_index);

    for (std::size_t i = 0; i < gauge_config->screen_configurations.size(); i++) {
        zassert_equal(saved_gauge_configuration.screen_configurations[i].id, gauge_config->screen_configurations[i].id);
        zassert_equal(saved_gauge_configuration.screen_configurations[i].grid.snap_enabled, gauge_config->screen_configurations[i].grid.snap_enabled);
        zassert_equal(saved_gauge_configuration.screen_configurations[i].grid.width, gauge_config->screen_configurations[i].grid.width);
        zassert_equal(saved_gauge_configuration.screen_configurations[i].grid.height, gauge_config->screen_configurations[i].grid.height);
        zassert_equal(saved_gauge_configuration.screen_configurations[i].grid.spacing_px, gauge_config->screen_configurations[i].grid.spacing_px);

        for (std::size_t j = 0; j < gauge_config->screen_configurations[i].widget_configurations.size(); j++) {
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[j].type, gauge_config->screen_configurations[i].widget_configurations[j].type);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[j].id, gauge_config->screen_configurations[i].widget_configurations[j].id);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[j].position_grid.x, gauge_config->screen_configurations[i].widget_configurations[j].position_grid.x);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[j].position_grid.y, gauge_config->screen_configurations[i].widget_configurations[j].position_grid.y);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[j].size_grid.width, gauge_config->screen_configurations[i].widget_configurations[j].size_grid.width);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[i].size_grid.height, gauge_config->screen_configurations[i].widget_configurations[i].size_grid.height);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[i].is_animation_enabled, gauge_config->screen_configurations[i].widget_configurations[i].is_animation_enabled);
            zassert_equal(saved_gauge_configuration.screen_configurations[i].widget_configurations[i].properties.size(), gauge_config->screen_configurations[i].widget_configurations[i].properties.size());

            for (auto& property : gauge_config->screen_configurations[i].widget_configurations[j].properties) {
                zassert_true(saved_gauge_configuration.screen_configurations[i].widget_configurations[j].properties[property.first] == gauge_config->screen_configurations[i].widget_configurations[j].properties[property.first]);
            }
        }
    }
}

#include <memory>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "configuration/ui_config/ui_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "domain/ui_domain/configuration/ui_configuration_manager.h"
#include "domain/ui_domain/models/widget_property.h"

#include "subsys/device_tree/dt_fs.h"
#include "subsys/fs/services/fs_service.h"

#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets::indicators;

ZTEST_SUITE(ui_configuration_manager, NULL, NULL, NULL, NULL, NULL);

std::shared_ptr<UiConfiguration> ui_configuration_manager_test_SetupTestUiConfig() {
    auto ui_configuration = make_shared_ext<UiConfiguration>();
    ui_configuration->active_screen_index = 8;

    ScreenConfiguration screen_configuration {
        .id = 8,
        .type = ScreenType::Gauge,
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
                .properties = {
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE), 0 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE), 100 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID), "2348664336" },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT), 35 },
                    { WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE), static_cast<std::uint16_t>(HorizontalChartIndicatorType::Line) }
                }
            }
        }
    };

    ui_configuration->screen_configurations.push_back(screen_configuration);

    return ui_configuration;
}

ZTEST(ui_configuration_manager, test_UiConfigurationManager_Save_config_successfully_saved) {
    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();

    auto ui_configuration_service = make_unique_ext<CborConfigurationService<UiConfig>>("ui_config", fs_service);
    auto ui_configuration_manager = std::make_shared<UiConfigurationManager>(std::move(ui_configuration_service));

    auto ui_config = ui_configuration_manager_test_SetupTestUiConfig();
    bool result = ui_configuration_manager->Update(ui_config);
    zassert_true(result);
}

ZTEST(ui_configuration_manager, test_UiConfigurationManager_Save_config_and_Load) {
    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();

    auto ui_configuration_service = make_unique_ext<CborConfigurationService<UiConfig>>("ui_config", fs_service);
    auto ui_configuration_manager = std::make_shared<UiConfigurationManager>(std::move(ui_configuration_service));

    auto ui_config = ui_configuration_manager_test_SetupTestUiConfig();
    bool result = ui_configuration_manager->Update(ui_config);
    zassert_true(result);

    auto saved_ui_configuration = *ui_configuration_manager->Get(true);

    zassert_equal(saved_ui_configuration.active_screen_index, ui_config->active_screen_index);

    for (std::size_t i = 0; i < ui_config->screen_configurations.size(); i++) {
        zassert_equal(saved_ui_configuration.screen_configurations[i].id, ui_config->screen_configurations[i].id);
        zassert_equal(saved_ui_configuration.screen_configurations[i].type, ui_config->screen_configurations[i].type);
        zassert_equal(saved_ui_configuration.screen_configurations[i].grid.snap_enabled, ui_config->screen_configurations[i].grid.snap_enabled);
        zassert_equal(saved_ui_configuration.screen_configurations[i].grid.width, ui_config->screen_configurations[i].grid.width);
        zassert_equal(saved_ui_configuration.screen_configurations[i].grid.height, ui_config->screen_configurations[i].grid.height);
        zassert_equal(saved_ui_configuration.screen_configurations[i].grid.spacing_px, ui_config->screen_configurations[i].grid.spacing_px);

        for (std::size_t j = 0; j < ui_config->screen_configurations[i].widget_configurations.size(); j++) {
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[j].type, ui_config->screen_configurations[i].widget_configurations[j].type);
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[j].id, ui_config->screen_configurations[i].widget_configurations[j].id);
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[j].position_grid.x, ui_config->screen_configurations[i].widget_configurations[j].position_grid.x);
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[j].position_grid.y, ui_config->screen_configurations[i].widget_configurations[j].position_grid.y);
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[j].size_grid.width, ui_config->screen_configurations[i].widget_configurations[j].size_grid.width);
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[i].size_grid.height, ui_config->screen_configurations[i].widget_configurations[i].size_grid.height);
            zassert_equal(saved_ui_configuration.screen_configurations[i].widget_configurations[i].properties.size(), ui_config->screen_configurations[i].widget_configurations[i].properties.size());

            for (auto& property : ui_config->screen_configurations[i].widget_configurations[j].properties) {
                zassert_true(saved_ui_configuration.screen_configurations[i].widget_configurations[j].properties[property.first] == ui_config->screen_configurations[i].widget_configurations[j].properties[property.first]);
            }
        }
    }
}

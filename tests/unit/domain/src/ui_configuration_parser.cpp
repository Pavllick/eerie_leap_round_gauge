#include <memory>
#include <fstream>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "configuration/services/cbor_configuration_service.h"
#include "configuration/services/json_configuration_service.h"

#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/configuration/parsers/ui_configuration_cbor_parser.h"
#include "domain/ui_domain/configuration/parsers/ui_configuration_json_parser.h"

#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::configuration::parsers;
using namespace eerie_leap::views::widgets::indicators;

ZTEST_SUITE(ui_configuration_parser, NULL, NULL, NULL, NULL, NULL);

pmr_unique_ptr<UiConfiguration> ui_configuration_parser_GetTestUiConfiguration() {
    auto ui_configuration = make_unique_pmr<UiConfiguration>(Mrm::GetDefaultPmr());
    ui_configuration->active_screen_index = 8;

    auto screen_configuration = make_shared_pmr<ScreenConfiguration>(Mrm::GetDefaultPmr());
    screen_configuration->id = 8;
    screen_configuration->type = ScreenType::Gauge;

    screen_configuration->grid.snap_enabled = true;
    screen_configuration->grid.width = 3;
    screen_configuration->grid.height = 3;
    screen_configuration->grid.spacing_px = 0;

    // First widget
    auto widget1 = make_shared_pmr<WidgetConfiguration>(Mrm::GetDefaultPmr());
    widget1->type = WidgetType::IndicatorArcFill;
    widget1->id = 0;
    widget1->position_grid.x = 0;
    widget1->position_grid.y = 0;
    widget1->size_grid.width = 3;
    widget1->size_grid.height = 3;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    screen_configuration->widget_configurations.push_back(std::move(widget1));

    // Second widget
    auto widget2 = make_shared_pmr<WidgetConfiguration>(Mrm::GetDefaultPmr());
    widget2->type = WidgetType::IndicatorDigital;
    widget2->id = 1;
    widget2->position_grid.x = 1;
    widget2->position_grid.y = 1;
    widget2->size_grid.width = 1;
    widget2->size_grid.height = 1;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    screen_configuration->widget_configurations.push_back(std::move(widget2));

    // Third widget
    auto widget3 = make_shared_pmr<WidgetConfiguration>(Mrm::GetDefaultPmr());
    widget3->type = WidgetType::IndicatorHorizontalChart;
    widget3->id = 2;
    widget3->position_grid.x = 0;
    widget3->position_grid.y = 0;
    widget3->size_grid.width = 3;
    widget3->size_grid.height = 1;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT)] = 35;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE)] = static_cast<std::uint16_t>(HorizontalChartIndicatorType::Line);
    screen_configuration->widget_configurations.push_back(std::move(widget3));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration));

    return ui_configuration;
}

void ui_configuration_parser_CompareUiConfigurations(UiConfiguration& ui_configuration, UiConfiguration& deserialized_ui_configuration) {
    zassert_equal(deserialized_ui_configuration.active_screen_index, ui_configuration.active_screen_index);

    for(std::size_t i = 0; i < ui_configuration.screen_configurations.size(); i++) {
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->id, ui_configuration.screen_configurations[i]->id);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->type, ui_configuration.screen_configurations[i]->type);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->grid.snap_enabled, ui_configuration.screen_configurations[i]->grid.snap_enabled);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->grid.width, ui_configuration.screen_configurations[i]->grid.width);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->grid.height, ui_configuration.screen_configurations[i]->grid.height);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->grid.spacing_px, ui_configuration.screen_configurations[i]->grid.spacing_px);

        for(std::size_t j = 0; j < ui_configuration.screen_configurations[i]->widget_configurations.size(); j++) {
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->type, ui_configuration.screen_configurations[i]->widget_configurations[j]->type);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->id, ui_configuration.screen_configurations[i]->widget_configurations[j]->id);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->position_grid.x, ui_configuration.screen_configurations[i]->widget_configurations[j]->position_grid.x);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->position_grid.y, ui_configuration.screen_configurations[i]->widget_configurations[j]->position_grid.y);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->size_grid.width, ui_configuration.screen_configurations[i]->widget_configurations[j]->size_grid.width);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->size_grid.height, ui_configuration.screen_configurations[i]->widget_configurations[j]->size_grid.height);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->properties.size(), ui_configuration.screen_configurations[i]->widget_configurations[j]->properties.size());
            for(auto& property : ui_configuration.screen_configurations[i]->widget_configurations[j]->properties) {
                zassert_true(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->properties[property.first] == ui_configuration.screen_configurations[i]->widget_configurations[j]->properties[property.first]);
            }
        }
    }
}

ZTEST(ui_configuration_parser, test_CborSerializeDeserialize) {
    UiConfigurationCborParser ui_configuration_cbor_parser;

    auto ui_configuration = ui_configuration_parser_GetTestUiConfiguration();

    auto serialized_ui_configuration = ui_configuration_cbor_parser.Serialize(*ui_configuration);
    auto deserialized_ui_configuration = ui_configuration_cbor_parser.Deserialize(Mrm::GetDefaultPmr(), *serialized_ui_configuration.get());

    ui_configuration_parser_CompareUiConfigurations(
        *ui_configuration, *deserialized_ui_configuration);
}

ZTEST(ui_configuration_parser, test_JsonSerializeDeserialize) {
    UiConfigurationJsonParser ui_configuration_json_parser;

    auto ui_configuration = ui_configuration_parser_GetTestUiConfiguration();

    auto serialized_ui_configuration = ui_configuration_json_parser.Serialize(*ui_configuration);
    auto deserialized_ui_configuration = ui_configuration_json_parser.Deserialize(Mrm::GetDefaultPmr(), *serialized_ui_configuration.get());

    ui_configuration_parser_CompareUiConfigurations(
        *ui_configuration, *deserialized_ui_configuration);
}

ZTEST(ui_configuration_parser, test_JsonSaveFile) {
    UiConfigurationJsonParser ui_configuration_json_parser;
    auto ui_config_json_serializer = std::make_unique<JsonSerializer<JsonUiConfig>>();

    auto ui_configuration = ui_configuration_parser_GetTestUiConfiguration();
    auto ui_config = ui_configuration_json_parser.Serialize(*ui_configuration);
    auto json = ui_config_json_serializer->Serialize(*ui_config);

    // Will create file in the twister-out directory
    std::ofstream file("../../../../../../../ui_config.json", std::ios::binary);
    file.write(json.c_str(), json.size());
    file.close();
}

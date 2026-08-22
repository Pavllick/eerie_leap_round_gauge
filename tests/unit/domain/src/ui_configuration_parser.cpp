#include <memory>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <eerie_memory.hpp>

#include "configuration/services/cbor_configuration_service.h"

#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/configuration/parsers/ui_configuration_cbor_parser.h"

#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

using namespace eerie_memory;
using namespace eerie_leap::configuration::services;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::configuration::parsers;
using namespace eerie_leap::views::widgets::indicators;

ZTEST_SUITE(ui_configuration_parser, NULL, NULL, NULL, NULL, NULL);

pmr_unique_ptr<UiConfiguration> ui_configuration_parser_GetTestUiConfiguration() {
    auto ui_configuration = make_unique_pmr<UiConfiguration>(Mrm::GetDefaultPmr());
    ui_configuration->active_screen_group_id = 3;

    auto screen_configuration = make_shared_pmr<ScreenConfiguration>(Mrm::GetDefaultPmr());
    screen_configuration->id = 8;
    screen_configuration->group_id = 3;
    screen_configuration->type = ScreenType::Gauge;
    screen_configuration->z_index = -2;
    screen_configuration->is_visible = true;

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
    widget1->z_index = -1;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget1->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    screen_configuration->AddWidget(std::move(widget1));

    // Second widget
    auto widget2 = make_shared_pmr<WidgetConfiguration>(Mrm::GetDefaultPmr());
    widget2->type = WidgetType::IndicatorDigital;
    widget2->id = 1;
    widget2->position_grid.x = 1;
    widget2->position_grid.y = 1;
    widget2->size_grid.width = 1;
    widget2->size_grid.height = 1;
    widget2->is_visible = false;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget2->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    screen_configuration->AddWidget(std::move(widget2));

    // Third widget
    auto widget3 = make_shared_pmr<WidgetConfiguration>(Mrm::GetDefaultPmr());
    widget3->type = WidgetType::IndicatorHorizontalChart;
    widget3->id = 2;
    widget3->position_grid.x = 0;
    widget3->position_grid.y = 0;
    widget3->size_grid.width = 3;
    widget3->size_grid.height = 1;
    widget3->z_index = 2;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT)] = 35;
    widget3->properties[WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE)] = static_cast<std::uint16_t>(HorizontalChartIndicatorType::Line);
    screen_configuration->AddWidget(std::move(widget3));

    ui_configuration->screen_configurations.push_back(std::move(screen_configuration));

    return ui_configuration;
}

void ui_configuration_parser_CompareUiConfigurations(UiConfiguration& ui_configuration, UiConfiguration& deserialized_ui_configuration) {
    zassert_equal(deserialized_ui_configuration.active_screen_group_id, ui_configuration.active_screen_group_id);

    for(std::size_t i = 0; i < ui_configuration.screen_configurations.size(); i++) {
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->id, ui_configuration.screen_configurations[i]->id);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->group_id, ui_configuration.screen_configurations[i]->group_id);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->type, ui_configuration.screen_configurations[i]->type);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->z_index, ui_configuration.screen_configurations[i]->z_index);
        zassert_equal(deserialized_ui_configuration.screen_configurations[i]->is_visible, ui_configuration.screen_configurations[i]->is_visible);
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
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->z_index, ui_configuration.screen_configurations[i]->widget_configurations[j]->z_index);
            zassert_equal(deserialized_ui_configuration.screen_configurations[i]->widget_configurations[j]->is_visible, ui_configuration.screen_configurations[i]->widget_configurations[j]->is_visible);
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

ZTEST(ui_configuration_parser, test_CborDeserializeOrdersWidgetsByZIndex) {
    UiConfigurationCborParser ui_configuration_cbor_parser;

    auto ui_configuration = ui_configuration_parser_GetTestUiConfiguration();
    auto& widget_configurations = ui_configuration->screen_configurations[0]->widget_configurations;
    widget_configurations[0]->z_index = 5;
    widget_configurations[1]->z_index = 1;
    widget_configurations[2]->z_index = 1;

    auto serialized_ui_configuration = ui_configuration_cbor_parser.Serialize(*ui_configuration);
    auto deserialized_ui_configuration = ui_configuration_cbor_parser.Deserialize(Mrm::GetDefaultPmr(), *serialized_ui_configuration.get());

    auto& deserialized_widget_configurations = deserialized_ui_configuration->screen_configurations[0]->widget_configurations;
    zassert_equal(deserialized_widget_configurations.size(), widget_configurations.size());

    // Equal z-index keeps configuration order.
    zassert_equal(deserialized_widget_configurations[0]->id, widget_configurations[1]->id);
    zassert_equal(deserialized_widget_configurations[1]->id, widget_configurations[2]->id);
    zassert_equal(deserialized_widget_configurations[2]->id, widget_configurations[0]->id);
}

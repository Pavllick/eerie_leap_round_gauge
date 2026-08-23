#include <array>
#include <memory>
#include <stdexcept>
#include <utility>

#include <zephyr/ztest.h>
#include <eerie_memory.hpp>

#include "utilities/memory/memory_resource_manager.h"

#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/configuration/parsers/ui_configuration_validator.h"

using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::ui_domain::configuration::parsers;

ZTEST_SUITE(ui_configuration_validator, NULL, NULL, NULL, NULL, NULL);

namespace {

std::shared_ptr<WidgetConfiguration> MakeWidget(uint32_t id) {
    auto widget_configuration = make_shared_pmr<WidgetConfiguration>(Mrm::GetDefaultPmr());
    widget_configuration->type = WidgetType::IndicatorDigital;
    widget_configuration->id = id;
    widget_configuration->position_grid.x = 0;
    widget_configuration->position_grid.y = 0;
    widget_configuration->size_grid.width = 3;
    widget_configuration->size_grid.height = 3;

    return widget_configuration;
}

std::shared_ptr<ScreenConfiguration> MakeScreen(uint32_t id, uint32_t group_id) {
    auto screen_configuration = make_shared_pmr<ScreenConfiguration>(Mrm::GetDefaultPmr());
    screen_configuration->id = id;
    screen_configuration->group_id = group_id;
    screen_configuration->type = ScreenType::Gauge;
    screen_configuration->grid.snap_enabled = true;
    screen_configuration->grid.width = 3;
    screen_configuration->grid.height = 3;
    screen_configuration->grid.spacing_px = 0;

    screen_configuration->AddWidget(MakeWidget(0));

    return screen_configuration;
}

pmr_unique_ptr<UiConfiguration> MakeConfiguration() {
    auto configuration = make_unique_pmr<UiConfiguration>(Mrm::GetDefaultPmr());
    configuration->active_screen_group_id = 1;
    configuration->screen_configurations.push_back(MakeScreen(0, 1));

    return configuration;
}

bool Validates(const UiConfiguration& configuration) {
    try {
        UiConfigurationValidator::Validate(configuration);
    } catch(const std::invalid_argument&) {
        return false;
    }

    return true;
}

} // namespace

ZTEST(ui_configuration_validator, test_valid_configuration) {
    zassert_true(Validates(*MakeConfiguration()));
}

ZTEST(ui_configuration_validator, test_empty_configuration_is_valid) {
    // The default configuration created on first boot has no screens.
    auto configuration = make_unique_pmr<UiConfiguration>(Mrm::GetDefaultPmr());

    zassert_true(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_too_many_screens_is_invalid) {
    auto configuration = MakeConfiguration();

    for(uint32_t i = 1; i <= 10; i++)
        configuration->screen_configurations.push_back(MakeScreen(i, 1));

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_duplicate_screen_id_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations.push_back(MakeScreen(0, 1));

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_unknown_screen_type_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->type = ScreenType::None;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_empty_grid_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->grid.width = 0;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_active_screen_group_without_screen_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->active_screen_group_id = 7;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_duplicate_widget_id_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->AddWidget(MakeWidget(0));

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_unknown_widget_type_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->widget_configurations[0]->type = WidgetType::None;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_empty_widget_size_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->widget_configurations[0]->size_grid.height = 0;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_widget_larger_than_grid_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->widget_configurations[0]->size_grid.width = 4;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_negative_widget_position_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->widget_configurations[0]->position_grid.y = -1;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_widget_position_outside_grid_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->widget_configurations[0]->position_grid.x = 3;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_known_widget_properties_are_valid) {
    auto configuration = MakeConfiguration();
    auto& properties = configuration->screen_configurations[0]->widget_configurations[0]->properties;

    properties[WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID)] = "sensor_1";
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED)] = true;
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = 0;
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE)] = 100.5;

    zassert_true(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_unknown_widget_property_is_invalid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->widget_configurations[0]->properties["NOT_A_PROPERTY"] = 1;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_widget_property_with_wrong_value_type_is_invalid) {
    auto configuration = MakeConfiguration();
    auto& properties = configuration->screen_configurations[0]->widget_configurations[0]->properties;

    properties[WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE)] = true;

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_widget_property_without_value_is_invalid) {
    auto configuration = MakeConfiguration();
    auto& properties = configuration->screen_configurations[0]->widget_configurations[0]->properties;

    properties[WidgetProperty::GetTypeName(WidgetPropertyType::LABEL)] = ConfigValue{};

    zassert_false(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_settings_screen_type_is_valid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->type = ScreenType::Settings;

    zassert_true(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_popup_screen_type_is_valid) {
    auto configuration = MakeConfiguration();
    configuration->screen_configurations[0]->type = ScreenType::Popup;

    zassert_true(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_control_widget_types_are_valid) {
    constexpr std::array control_types = {
        WidgetType::ControlSlider,
        WidgetType::ControlToggle,
        WidgetType::ControlButton,
        WidgetType::IndicatorSetting
    };

    for(auto control_type : control_types) {
        auto configuration = MakeConfiguration();
        configuration->screen_configurations[0]->widget_configurations[0]->type = control_type;

        zassert_true(Validates(*configuration), "Expected widget type %u to be accepted.", static_cast<uint32_t>(control_type));
    }
}

ZTEST(ui_configuration_validator, test_control_widget_properties_are_valid) {
    auto configuration = MakeConfiguration();
    auto& widget_configuration = configuration->screen_configurations[0]->widget_configurations[0];
    widget_configuration->type = WidgetType::ControlSlider;

    auto& properties = widget_configuration->properties;
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::SETTING_ID)] = "display.brightness";
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::UNIT)] = "%";
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::STEP)] = 5.0;
    properties[WidgetProperty::GetTypeName(WidgetPropertyType::TARGET_GROUP)] = 2;

    zassert_true(Validates(*configuration));
}

ZTEST(ui_configuration_validator, test_numeric_setting_id_is_invalid) {
    auto configuration = MakeConfiguration();
    auto& properties = configuration->screen_configurations[0]->widget_configurations[0]->properties;

    properties[WidgetProperty::GetTypeName(WidgetPropertyType::SETTING_ID)] = 1;

    zassert_false(Validates(*configuration));
}

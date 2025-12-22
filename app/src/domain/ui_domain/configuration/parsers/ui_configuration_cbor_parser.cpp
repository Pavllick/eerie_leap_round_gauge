#include <utility>

#include "utilities/cbor/cbor_helpers.hpp"

// #include "ui_configuration_validator.h"
#include "ui_configuration_cbor_parser.h"

namespace eerie_leap::domain::ui_domain::configuration::parsers {

using namespace eerie_leap::utilities::cbor;

pmr_unique_ptr<CborUiConfig> UiConfigurationCborParser::Serialize(const UiConfiguration& configuration) {
    // UiConfigurationValidator::Validate(configuration);

    auto config = make_unique_pmr<CborUiConfig>(Mrm::GetExtPmr());

    config->active_screen_index = configuration.active_screen_index;

    config->properties_present = configuration.properties.size() > 0;
    if(configuration.properties.size() > 0)
        ValueTypeToCborPropertyValueType(config->properties, configuration.properties);

    config->CborScreenConfig_m.clear();
    for(int i = 0; i < configuration.screen_configurations.size(); i++) {
        CborScreenConfig screen_config(std::allocator_arg, Mrm::GetExtPmr());;

        screen_config.id = configuration.screen_configurations[i]->id;
        screen_config.type = static_cast<uint32_t>(configuration.screen_configurations[i]->type);
        screen_config.grid.snap_enabled = configuration.screen_configurations[i]->grid.snap_enabled;
        screen_config.grid.width = configuration.screen_configurations[i]->grid.width;
        screen_config.grid.height = configuration.screen_configurations[i]->grid.height;
        screen_config.grid.spacing_px = configuration.screen_configurations[i]->grid.spacing_px;

        screen_config.CborWidgetConfig_m.clear();
        for(int j = 0; j < configuration.screen_configurations[i]->widget_configurations.size(); j++) {
            CborWidgetConfig widget_config(std::allocator_arg, Mrm::GetExtPmr());;

            widget_config.id = configuration.screen_configurations[i]->widget_configurations[j]->id;
            widget_config.type = static_cast<uint32_t>(configuration.screen_configurations[i]->widget_configurations[j]->type);
            widget_config.position.x = configuration.screen_configurations[i]->widget_configurations[j]->position_grid.x;
            widget_config.position.y = configuration.screen_configurations[i]->widget_configurations[j]->position_grid.y;
            widget_config.size.width = configuration.screen_configurations[i]->widget_configurations[j]->size_grid.width;
            widget_config.size.height = configuration.screen_configurations[i]->widget_configurations[j]->size_grid.height;

            widget_config.properties_present = configuration.screen_configurations[i]->widget_configurations[j]->properties.size() > 0;
            if(configuration.screen_configurations[i]->widget_configurations[j]->properties.size() > 0)
                ValueTypeToCborPropertyValueType(widget_config.properties, configuration.screen_configurations[i]->widget_configurations[j]->properties);
            screen_config.CborWidgetConfig_m.push_back(std::move(widget_config));
        }

        config->CborScreenConfig_m.push_back(std::move(screen_config));
    }

    return config;
}

pmr_unique_ptr<UiConfiguration> UiConfigurationCborParser::Deserialize(
    std::pmr::memory_resource* mr,
    const CborUiConfig& config) {

    auto configuration = make_unique_pmr<UiConfiguration>(mr);

    configuration->active_screen_index = config.active_screen_index;

    if(config.properties_present)
        CborPropertyValueTypeToValueType(mr, configuration->properties, config.properties);

    for(int i = 0; i < config.CborScreenConfig_m.size(); i++) {
        auto screen_configuration = make_shared_pmr<ScreenConfiguration>(mr);
        screen_configuration->id = config.CborScreenConfig_m[i].id;
        screen_configuration->type = static_cast<ScreenType>(config.CborScreenConfig_m[i].type);

        screen_configuration->grid.snap_enabled = config.CborScreenConfig_m[i].grid.snap_enabled;
        screen_configuration->grid.width = config.CborScreenConfig_m[i].grid.width;
        screen_configuration->grid.height = config.CborScreenConfig_m[i].grid.height;
        screen_configuration->grid.spacing_px = config.CborScreenConfig_m[i].grid.spacing_px;

        for(int j = 0; j < config.CborScreenConfig_m[i].CborWidgetConfig_m.size(); j++) {
            auto widget_configuration = make_shared_pmr<WidgetConfiguration>(mr);
            widget_configuration->type = static_cast<WidgetType>(config.CborScreenConfig_m[i].CborWidgetConfig_m[j].type);
            widget_configuration->id = config.CborScreenConfig_m[i].CborWidgetConfig_m[j].id;
            widget_configuration->position_grid.x = config.CborScreenConfig_m[i].CborWidgetConfig_m[j].position.x;
            widget_configuration->position_grid.y = config.CborScreenConfig_m[i].CborWidgetConfig_m[j].position.y;
            widget_configuration->size_grid.width = config.CborScreenConfig_m[i].CborWidgetConfig_m[j].size.width;
            widget_configuration->size_grid.height = config.CborScreenConfig_m[i].CborWidgetConfig_m[j].size.height;

            if(config.CborScreenConfig_m[i].CborWidgetConfig_m[j].properties_present)
                CborPropertyValueTypeToValueType(mr, widget_configuration->properties, config.CborScreenConfig_m[i].CborWidgetConfig_m[j].properties);
            screen_configuration->widget_configurations.push_back(std::move(widget_configuration));
        }

        configuration->screen_configurations.push_back(std::move(screen_configuration));
    }

    // UiConfigurationValidator::Validate(*configuration);

    return configuration;
}

void UiConfigurationCborParser::ValueTypeToCborPropertyValueType(CborPropertiesConfig& properties_config, const std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties) {
    size_t i = 0;
    for(auto& [key, value] : properties) {
        CborPropertiesConfig_CborPropertyValueType_m property_value(std::allocator_arg, Mrm::GetExtPmr());
        property_value.CborPropertyValueType_m_key = CborHelpers::ToZcborString(key);

        auto& prop = property_value.CborPropertyValueType_m;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_int_c;
                prop.value = static_cast<int32_t>(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_float_c;
                prop.value = static_cast<double>(arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_tstr_c;
                prop.value = CborHelpers::ToZcborString(arg);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_bool_c;
                prop.value = arg;
            } else if constexpr (std::is_same_v<T, std::pmr::vector<int>>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_int_l_c;

                prop.value = std::pmr::vector<int32_t>(Mrm::GetExtPmr());
                for (auto it = arg.begin(); it != arg.end(); ++it)
                    std::get<std::pmr::vector<int32_t>>(prop.value).push_back(*it);
            } else if constexpr (std::is_same_v<T, std::pmr::vector<std::pmr::string>>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_tstr_l_c;

                prop.value = std::pmr::vector<zcbor_string>(Mrm::GetExtPmr());
                for (auto it = arg.begin(); it != arg.end(); ++it)
                    std::get<std::pmr::vector<zcbor_string>>(prop.value).push_back(CborHelpers::ToZcborString(*it));
            } else if constexpr (std::is_same_v<T, std::pmr::unordered_map<std::pmr::string, std::pmr::string>>) {
                prop.CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_map_c;

                prop.value = std::pmr::vector<map_tstrtstr>(Mrm::GetExtPmr());
                for (auto it = arg.begin(); it != arg.end(); ++it) {
                    std::get<std::pmr::vector<map_tstrtstr>>(prop.value).push_back({
                        .tstrtstr_key = CborHelpers::ToZcborString(it->first),
                        .tstrtstr = CborHelpers::ToZcborString(it->second)
                    });
                }
            } else {
                throw std::runtime_error("Unsupported property value type");
            }
        }, value);

        properties_config.CborPropertyValueType_m.push_back(std::move(property_value));

        ++i;
    }
}

void UiConfigurationCborParser::CborPropertyValueTypeToValueType(
    std::pmr::memory_resource* mr,
    std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties,
    const CborPropertiesConfig& properties_config) {

    if(properties_config.CborPropertyValueType_m.size() == 0)
        return;

    for(auto& property : properties_config.CborPropertyValueType_m) {
        ConfigValue value;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int32_t>) {
                value = arg;
            } else if constexpr (std::is_same_v<T, double>) {
                value = static_cast<double>(arg);
            } else if constexpr (std::is_same_v<T, zcbor_string>) {
                value = CborHelpers::ToPmrString(mr, arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                value = arg;
            } else if constexpr (std::is_same_v<T, std::pmr::vector<int32_t>>) {
                value = std::pmr::vector<int>(mr);
                for (const auto& it : arg)
                    std::get<std::pmr::vector<int>>(value).push_back(it);
            } else if constexpr (std::is_same_v<T, std::pmr::vector<zcbor_string>>) {
                value = std::pmr::vector<std::pmr::string>(mr);
                for (const auto& it : arg)
                    std::get<std::pmr::vector<std::pmr::string>>(value).push_back(CborHelpers::ToPmrString(mr, it));
            } else if constexpr (std::is_same_v<T, std::pmr::unordered_map<zcbor_string, zcbor_string>>) {
                value = std::pmr::unordered_map<std::pmr::string, std::pmr::string>(mr);
                for (const auto& it : arg) {
                    std::get<std::pmr::unordered_map<std::pmr::string, std::pmr::string>>(value).insert({
                        CborHelpers::ToPmrString(mr, it.tstrtstr_key),
                        CborHelpers::ToPmrString(mr, it.tstrtstr)
                    });
                }
            } else {
                throw std::runtime_error("Unsupported property value type");
            }
        }, property.CborPropertyValueType_m.value);

        properties.emplace(CborHelpers::ToPmrString(mr, property.CborPropertyValueType_m_key), std::move(value));
    }
}

} // namespace eerie_leap::domain::ui_domain::configuration::parsers

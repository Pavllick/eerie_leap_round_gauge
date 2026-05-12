#include "utilities/memory/memory_resource_manager.h"

#include "ui_configuration_json_parser.h"

namespace eerie_leap::domain::ui_domain::configuration::parsers {

namespace json = boost::json;
using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::configuration::json::configs;
using namespace eerie_leap::domain::ui_domain::models;

pmr_unique_ptr<JsonUiConfig> UiConfigurationJsonParser::Serialize(const UiConfiguration& configuration) {
    auto config = make_unique_pmr<JsonUiConfig>(Mrm::GetExtPmr());

    config->version = 1; // Set appropriate version
    config->active_screen_index = configuration.active_screen_index;

    if(configuration.properties.size() > 0)
        ConfigValueToJsonPropertyValue(config->properties, configuration.properties);

    config->screens.clear();
    for(size_t i = 0; i < configuration.screen_configurations.size(); i++) {
        JsonScreenConfig screen_config(Mrm::GetBoostExtPmr());

        screen_config.id = configuration.screen_configurations[i]->id;
        screen_config.type = json::string(
            std::to_string(static_cast<uint32_t>(configuration.screen_configurations[i]->type)),
            Mrm::GetBoostExtPmr()
        );

        screen_config.grid.snap_enabled = configuration.screen_configurations[i]->grid.snap_enabled;
        screen_config.grid.width = configuration.screen_configurations[i]->grid.width;
        screen_config.grid.height = configuration.screen_configurations[i]->grid.height;
        screen_config.grid.spacing_px = configuration.screen_configurations[i]->grid.spacing_px;

        screen_config.widgets.clear();
        for(size_t j = 0; j < configuration.screen_configurations[i]->widget_configurations.size(); j++) {
            JsonWidgetConfig widget_config(Mrm::GetBoostExtPmr());

            widget_config.id = configuration.screen_configurations[i]->widget_configurations[j]->id;
            widget_config.type = json::string(
                std::to_string(static_cast<uint32_t>(configuration.screen_configurations[i]->widget_configurations[j]->type)),
                Mrm::GetBoostExtPmr()
            );
            widget_config.position.x = configuration.screen_configurations[i]->widget_configurations[j]->position_grid.x;
            widget_config.position.y = configuration.screen_configurations[i]->widget_configurations[j]->position_grid.y;
            widget_config.size.width = configuration.screen_configurations[i]->widget_configurations[j]->size_grid.width;
            widget_config.size.height = configuration.screen_configurations[i]->widget_configurations[j]->size_grid.height;

            if(configuration.screen_configurations[i]->widget_configurations[j]->properties.size() > 0)
                ConfigValueToJsonPropertyValue(
                    widget_config.properties,
                    configuration.screen_configurations[i]->widget_configurations[j]->properties
                );

            screen_config.widgets.push_back(std::move(widget_config));
        }

        config->screens.push_back(std::move(screen_config));
    }

    return config;
}

pmr_unique_ptr<UiConfiguration> UiConfigurationJsonParser::Deserialize(
    std::pmr::memory_resource* mr,
    const JsonUiConfig& config) {

    auto configuration = make_unique_pmr<UiConfiguration>(mr);

    configuration->active_screen_index = config.active_screen_index;

    if(config.properties.properties.size() > 0)
        JsonPropertyValueToConfigValue(mr, configuration->properties, config.properties);

    for(size_t i = 0; i < config.screens.size(); i++) {
        auto screen_configuration = make_shared_pmr<ScreenConfiguration>(mr);
        screen_configuration->id = config.screens[i].id;
        screen_configuration->type = static_cast<ScreenType>(std::stoi(std::string(config.screens[i].type.c_str())));

        screen_configuration->grid.snap_enabled = config.screens[i].grid.snap_enabled;
        screen_configuration->grid.width = config.screens[i].grid.width;
        screen_configuration->grid.height = config.screens[i].grid.height;
        screen_configuration->grid.spacing_px = config.screens[i].grid.spacing_px;

        for(size_t j = 0; j < config.screens[i].widgets.size(); j++) {
            auto widget_configuration = make_shared_pmr<WidgetConfiguration>(mr);
            widget_configuration->type = static_cast<WidgetType>(std::stoi(std::string(config.screens[i].widgets[j].type.c_str())));
            widget_configuration->id = config.screens[i].widgets[j].id;
            widget_configuration->position_grid.x = config.screens[i].widgets[j].position.x;
            widget_configuration->position_grid.y = config.screens[i].widgets[j].position.y;
            widget_configuration->size_grid.width = config.screens[i].widgets[j].size.width;
            widget_configuration->size_grid.height = config.screens[i].widgets[j].size.height;

            if(config.screens[i].widgets[j].properties.properties.size() > 0)
                JsonPropertyValueToConfigValue(
                    mr,
                    widget_configuration->properties,
                    config.screens[i].widgets[j].properties
                );

            screen_configuration->widget_configurations.push_back(std::move(widget_configuration));
        }

        configuration->screen_configurations.push_back(std::move(screen_configuration));
    }

    return configuration;
}

void UiConfigurationJsonParser::ConfigValueToJsonPropertyValue(
    JsonPropertiesConfig& properties_config,
    const std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties) {

    for(const auto& [key, value] : properties) {
        json::string json_key(key.c_str(), Mrm::GetBoostExtPmr());
        JsonPropertyValue property_value(Mrm::GetBoostExtPmr());

        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                // Skip monostate
            } else if constexpr (std::is_same_v<T, int>) {
                property_value.type = JsonPropertyType::Int;
                property_value.value = static_cast<int64_t>(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                property_value.type = JsonPropertyType::Float;
                property_value.value = arg;
            } else if constexpr (std::is_same_v<T, std::pmr::string>) {
                property_value.type = JsonPropertyType::String;
                property_value.value = json::string(arg.c_str(), Mrm::GetBoostExtPmr());
            } else if constexpr (std::is_same_v<T, bool>) {
                property_value.type = JsonPropertyType::Bool;
                property_value.value = arg;
            } else if constexpr (std::is_same_v<T, std::pmr::vector<int>>) {
                property_value.type = JsonPropertyType::IntList;
                json::array arr(Mrm::GetBoostExtPmr());
                for (const auto& item : arg)
                    arr.push_back(static_cast<int64_t>(item));
                property_value.value = std::move(arr);
            } else if constexpr (std::is_same_v<T, std::pmr::vector<std::pmr::string>>) {
                property_value.type = JsonPropertyType::StringList;
                json::array arr(Mrm::GetBoostExtPmr());
                for (const auto& item : arg)
                    arr.push_back(json::string(item.c_str(), Mrm::GetBoostExtPmr()));
                property_value.value = std::move(arr);
            } else if constexpr (std::is_same_v<T, std::pmr::unordered_map<std::pmr::string, std::pmr::string>>) {
                property_value.type = JsonPropertyType::StringMap;
                json::object map_obj(Mrm::GetBoostExtPmr());
                for (const auto& [map_key, map_val] : arg)
                    map_obj[map_key.c_str()] = json::string(map_val.c_str(), Mrm::GetBoostExtPmr());
                property_value.value = std::move(map_obj);
            } else {
                throw std::runtime_error("Unsupported property value type");
            }
        }, value);

        properties_config.properties.push_back(std::make_pair(std::move(json_key), std::move(property_value)));
    }
}

void UiConfigurationJsonParser::JsonPropertyValueToConfigValue(
    std::pmr::memory_resource* mr,
    std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties,
    const JsonPropertiesConfig& properties_config) {

    if(properties_config.properties.size() == 0)
        return;

    for(const auto& [key, prop_value] : properties_config.properties) {
        ConfigValue value;

        switch(prop_value.type) {
            case JsonPropertyType::Int:
                value = static_cast<int>(prop_value.value.as_int64());
                break;

            case JsonPropertyType::Float:
                value = prop_value.value.as_double();
                break;

            case JsonPropertyType::String:
                value = std::pmr::string(prop_value.value.as_string().c_str(), mr);
                break;

            case JsonPropertyType::Bool:
                value = prop_value.value.as_bool();
                break;

            case JsonPropertyType::IntList: {
                const json::array& arr = prop_value.value.as_array();
                std::pmr::vector<int> vec(mr);
                vec.reserve(arr.size());
                for(const auto& item : arr)
                    vec.push_back(static_cast<int>(item.as_int64()));
                value = std::move(vec);
                break;
            }

            case JsonPropertyType::StringList: {
                const json::array& arr = prop_value.value.as_array();
                std::pmr::vector<std::pmr::string> vec(mr);
                vec.reserve(arr.size());
                for(const auto& item : arr)
                    vec.push_back(std::pmr::string(item.as_string().c_str(), mr));
                value = std::move(vec);
                break;
            }

            case JsonPropertyType::StringMap: {
                const json::object& obj = prop_value.value.as_object();
                std::pmr::unordered_map<std::pmr::string, std::pmr::string> map(mr);
                for(const auto& [map_key, map_val] : obj)
                    map.emplace(
                        std::pmr::string(map_key.data(), mr),
                        std::pmr::string(map_val.as_string().c_str(), mr)
                    );
                value = std::move(map);
                break;
            }

            default:
                throw std::runtime_error("Unknown property type");
        }

        properties.emplace(std::pmr::string(key.c_str(), mr), std::move(value));
    }
}

} // namespace eerie_leap::domain::ui_domain::configuration::parsers

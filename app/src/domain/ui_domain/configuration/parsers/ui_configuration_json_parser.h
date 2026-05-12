#pragma once

#include "eerie_memory.hpp"

#include "utilities/type/config_value.h"
#include "configuration/json/configs/json_ui_config.h"
#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration::parsers {

using eerie_leap::utilities::type::ConfigValue;
using eerie_leap::configuration::json::configs::JsonUiConfig;
using eerie_leap::configuration::json::configs::JsonPropertiesConfig;
using eerie_leap::domain::ui_domain::models::UiConfiguration;

class UiConfigurationJsonParser {
private:
    static void ConfigValueToJsonPropertyValue(
        JsonPropertiesConfig& properties_config,
        const std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties);
    static void JsonPropertyValueToConfigValue(
        std::pmr::memory_resource* mr,
        std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties,
        const JsonPropertiesConfig& properties_config);

public:
    UiConfigurationJsonParser() = default;

    eerie_memory::pmr_unique_ptr<JsonUiConfig> Serialize(const UiConfiguration& configuration);
    eerie_memory::pmr_unique_ptr<UiConfiguration> Deserialize(std::pmr::memory_resource* mr, const JsonUiConfig& config);
};

} // namespace eerie_leap::domain::ui_domain::configuration::parsers

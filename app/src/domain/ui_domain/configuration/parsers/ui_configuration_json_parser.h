#pragma once

#include "utilities/memory/memory_resource_manager.h"
#include "utilities/type/config_value.h"
#include "configuration/json/configs/json_ui_config.h"
#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration::parsers {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::type;
using namespace eerie_leap::configuration::json::configs;
using namespace eerie_leap::domain::ui_domain::models;

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

    pmr_unique_ptr<JsonUiConfig> Serialize(const UiConfiguration& configuration);
    pmr_unique_ptr<UiConfiguration> Deserialize(std::pmr::memory_resource* mr, const JsonUiConfig& config);
};

} // namespace eerie_leap::domain::ui_domain::configuration::parsers

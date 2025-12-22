#pragma once

#include "utilities/memory/memory_resource_manager.h"
#include "configuration/cbor/cbor_ui_config/cbor_ui_config.h"
#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration::parsers {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::ui_domain::models;

class UiConfigurationCborParser {
private:
    void ValueTypeToCborPropertyValueType(CborPropertiesConfig& properties_config, const std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties);
    void CborPropertyValueTypeToValueType(std::pmr::unordered_map<std::pmr::string, ConfigValue>& properties, const CborPropertiesConfig& properties_config);

public:
    UiConfigurationCborParser() = default;

    pmr_unique_ptr<CborUiConfig> Serialize(const UiConfiguration& configuration);
    pmr_unique_ptr<UiConfiguration> Deserialize(std::pmr::memory_resource* mr, const CborUiConfig& config);
};

} // namespace eerie_leap::domain::ui_domain::configuration::parsers

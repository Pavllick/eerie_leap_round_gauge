#pragma once

#include "utilities/memory/memory_resource_manager.h"
#include "configuration/cbor/cbor_system_config/cbor_system_config.h"
#include "domain/system_domain/models/system_configuration.h"

namespace eerie_leap::domain::system_domain::configuration::parsers {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::system_domain::models;

class SystemConfigurationCborParser {
public:
    SystemConfigurationCborParser() = default;

    pmr_unique_ptr<CborSystemConfig> Serialize(const SystemConfiguration& configuration);
    pmr_unique_ptr<SystemConfiguration> Deserialize(std::pmr::memory_resource* mr, const CborSystemConfig& config);
};

} // namespace eerie_leap::domain::system_domain::configuration::parsers

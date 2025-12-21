#pragma once

#include <memory>

#include "utilities/memory/memory_resource_manager.h"
#include "configuration/cbor/cbor_system_config/cbor_system_config.h"
#include "configuration/services/cbor_configuration_service.h"
#include "domain/system_domain/models/system_configuration.h"
#include "domain/system_domain/configuration/parsers/system_configuration_cbor_parser.h"

namespace eerie_leap::domain::system_domain::configuration {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::configuration::services;
using namespace eerie_leap::domain::system_domain::models;
using namespace eerie_leap::domain::system_domain::configuration::parsers;

class SystemConfigurationManager{
private:
    std::unique_ptr<CborConfigurationService<CborSystemConfig>> cbor_configuration_service_;

    std::unique_ptr<SystemConfigurationCborParser> cbor_parser_;

    std::shared_ptr<SystemConfiguration> configuration_;

    bool UpdateHwVersion(uint32_t hw_version);
    bool UpdateSwVersion(uint32_t sw_version);
    bool CreateDefaultConfiguration();

public:
    explicit SystemConfigurationManager(std::unique_ptr<CborConfigurationService<CborSystemConfig>> system_configuration_service);

    bool UpdateInterfaceChannel(uint16_t interface_channel);
    bool UpdateBuildNumber(uint32_t build_number);

    bool Update(const SystemConfiguration& configuration);
    std::shared_ptr<SystemConfiguration> Get(bool force_load = false);
};

} // eerie_leap::domain::system_domain::configuration

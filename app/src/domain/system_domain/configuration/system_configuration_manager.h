#pragma once

#include <memory>

#include "utilities/memory/heap_allocator.h"
#include "configuration/system_config/system_config.h"
#include "configuration/services/configuration_service.h"
#include "domain/system_domain/system_configuration.h"

namespace eerie_leap::domain::system_domain::configuration {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::configuration::services;
using namespace eerie_leap::domain::system_domain;

class SystemConfigurationManager{
private:
    ext_unique_ptr<ConfigurationService<SystemConfig>> system_configuration_service_;
    ext_unique_ptr<ExtVector> system_config_raw_;
    ext_unique_ptr<SystemConfig> system_config_;
    std::shared_ptr<SystemConfiguration> system_configuration_;

    bool UpdateHwVersion(uint32_t hw_version);
    bool UpdateSwVersion(uint32_t sw_version);
    bool CreateDefaultSystemConfiguration();

public:
    explicit SystemConfigurationManager(ext_unique_ptr<ConfigurationService<SystemConfig>> system_configuration_service);

    bool UpdateInterfaceChannel(uint16_t interface_channel);
    bool UpdateBuildNumber(uint32_t build_number);

    bool Update(std::shared_ptr<SystemConfiguration> system_configuration);
    std::shared_ptr<SystemConfiguration> Get(bool force_load = false);
};

} // eerie_leap::domain::system_domain::configuration

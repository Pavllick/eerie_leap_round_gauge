#pragma once

#include <memory>

#include "configuration/system_config/system_config.h"
#include "configuration/services/configuration_service.h"
#include "domain/system_domain/system_configuration.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::domain::system_domain;

class SystemConfigurationController {
private:
    std::shared_ptr<ConfigurationService<SystemConfig>> system_configuration_service_;
    std::shared_ptr<ExtVector> system_config_raw_;
    std::shared_ptr<SystemConfig> system_config_;
    std::shared_ptr<SystemConfiguration> system_configuration_;

public:
    explicit SystemConfigurationController(std::shared_ptr<ConfigurationService<SystemConfig>> system_configuration_service);

    bool Update(std::shared_ptr<SystemConfiguration> system_configuration);
    std::shared_ptr<SystemConfiguration> Get(bool force_load = false);
};

} // namespace eerie_leap::controllers

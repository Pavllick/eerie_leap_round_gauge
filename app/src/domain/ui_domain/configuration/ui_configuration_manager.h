#pragma once

#include <memory>

#include "utilities/memory/memory_resource_manager.h"
#include "configuration/cbor/ui_config/ui_config.h"
#include "configuration/services/configuration_service.h"

#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::ui_domain::models;

class UiConfigurationManager {
private:
    std::unique_ptr<ConfigurationService<UiConfig>> ui_configuration_service_;
    std::pmr::vector<uint8_t> ui_config_raw_;
    pmr_unique_ptr<UiConfig> ui_config_;
    std::shared_ptr<UiConfiguration> ui_configuration_;

public:
    explicit UiConfigurationManager(std::unique_ptr<ConfigurationService<UiConfig>> ui_configuration_service);

    bool Update(std::shared_ptr<UiConfiguration> ui_configuration);
    std::shared_ptr<UiConfiguration> Get(bool force_load = false);
};

} // namespace eerie_leap::domain::ui_domain::configuration

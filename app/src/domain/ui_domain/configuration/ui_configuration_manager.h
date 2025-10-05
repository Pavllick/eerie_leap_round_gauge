#pragma once

#include <memory>

#include "utilities/memory/heap_allocator.h"
#include "configuration/ui_config/ui_config.h"
#include "configuration/services/configuration_service.h"

#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::ui_domain::models;

class UiConfigurationManager {
private:
    ext_unique_ptr<ConfigurationService<UiConfig>> ui_configuration_service_;
    ext_unique_ptr<ExtVector> ui_config_raw_;
    ext_unique_ptr<UiConfig> ui_config_;
    std::shared_ptr<UiConfiguration> ui_configuration_;

public:
    explicit UiConfigurationManager(ext_unique_ptr<ConfigurationService<UiConfig>> ui_configuration_service);

    bool Update(std::shared_ptr<UiConfiguration> ui_configuration);
    std::shared_ptr<UiConfiguration> Get(bool force_load = false);
};

} // namespace eerie_leap::domain::ui_domain::configuration

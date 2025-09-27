#pragma once

#include <memory>

#include "utilities/memory/heap_allocator.h"
#include "configuration/gauge_config/gauge_config.h"
#include "configuration/services/configuration_service.h"

#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::configuration::services;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::ui_domain::models;

class UiConfigurationManager {
private:
    std::shared_ptr<ConfigurationService<GaugeConfig>> ui_configuration_service_;
    std::shared_ptr<ExtVector> gauge_config_raw_;
    std::shared_ptr<GaugeConfig> gauge_config_;
    std::shared_ptr<UiConfiguration> ui_configuration_;

public:
    explicit UiConfigurationManager(std::shared_ptr<ConfigurationService<GaugeConfig>> ui_configuration_service);

    bool Update(std::shared_ptr<UiConfiguration> ui_configuration);
    std::shared_ptr<UiConfiguration> Get(bool force_load = false);
};

} // namespace eerie_leap::domain::ui_domain::configuration

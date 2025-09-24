#pragma once

#include <memory>

#include "utilities/memory/heap_allocator.h"
#include "configuration/gauge_config/gauge_config.h"
#include "configuration/services/configuration_service.h"

#include "views/configuration/gauge_configuration.h"

using namespace eerie_leap::configuration::services;
namespace eerie_leap::controllers::configuration {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::views::configuration;

class GaugeConfigurationController {
private:
    std::shared_ptr<ConfigurationService<GaugeConfig>> gauge_configuration_service_;
    std::shared_ptr<ExtVector> gauge_config_raw_;
    std::shared_ptr<GaugeConfig> gauge_config_;
    std::shared_ptr<GaugeConfiguration> gauge_configuration_;

public:
    explicit GaugeConfigurationController(std::shared_ptr<ConfigurationService<GaugeConfig>> gauge_configuration_service);

    bool Update(std::shared_ptr<GaugeConfiguration> gauge_configuration);
    std::shared_ptr<GaugeConfiguration> Get(bool force_load = false);
};

} // namespace eerie_leap::controllers::configuration

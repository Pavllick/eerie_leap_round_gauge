#pragma once

#include <memory>

#include "configuration/cbor/cbor_display_config/cbor_display_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "domain/configuration_domain/utilities/i_cbor_configuration_manager.h"
#include "domain/configuration_domain/utilities/i_configuration_manager.h"
#include "domain/display_domain/configuration/parsers/display_configuration_cbor_parser.h"
#include "domain/display_domain/models/display_configuration.h"

namespace eerie_leap::domain::display_domain::configuration {

namespace config_services = eerie_leap::configuration::services;
using eerie_leap::domain::configuration_domain::utilities::ICborConfigurationManager;
using eerie_leap::domain::configuration_domain::utilities::IConfigurationManager;

using eerie_leap::domain::display_domain::models::DisplayConfiguration;
using eerie_leap::domain::display_domain::configuration::parsers::DisplayConfigurationCborParser;

class DisplayConfigurationManager : public ICborConfigurationManager, public IConfigurationManager {
private:
    std::unique_ptr<config_services::CborConfigurationService<CborDisplayConfig>> cbor_configuration_service_;
    std::unique_ptr<DisplayConfigurationCborParser> cbor_parser_;
    std::shared_ptr<DisplayConfiguration> configuration_;

    ConfigurationUpdatedHandler configuration_updated_handler_;

    bool CreateDefaultConfiguration();

public:
    explicit DisplayConfigurationManager(
        std::unique_ptr<config_services::CborConfigurationService<CborDisplayConfig>> cbor_configuration_service);

    void RegisterConfigurationUpdatedHandler(ConfigurationUpdatedHandler handler) override;

    bool Update(const DisplayConfiguration& configuration);
    std::shared_ptr<DisplayConfiguration> Get(bool force_load = false);

    bool ApplyCborConfiguration(std::span<const uint8_t> cbor_data) override;
    std::pmr::vector<uint8_t> GetCborConfiguration() override;
};

} // namespace eerie_leap::domain::display_domain::configuration

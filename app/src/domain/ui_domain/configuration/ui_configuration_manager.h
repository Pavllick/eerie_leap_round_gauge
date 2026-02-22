#pragma once

#include <memory>

#include "utilities/memory/memory_resource_manager.h"
#include "configuration/cbor/cbor_ui_config/cbor_ui_config.h"
#include "configuration/json/configs/json_ui_config.h"
#include "configuration/services/cbor_configuration_service.h"
#include "configuration/services/json_configuration_service.h"

#include "domain/configuration_domain/utilities/i_json_configuration_manager.h"
#include "domain/ui_domain/configuration/parsers/ui_configuration_cbor_parser.h"
#include "domain/ui_domain/configuration/parsers/ui_configuration_json_parser.h"

#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::configuration::services;
using namespace eerie_leap::configuration::json::configs;
using namespace eerie_leap::domain::configuration_domain::utilities;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::ui_domain::configuration::parsers;

class UiConfigurationManager : public IJsonConfigurationManager {
private:
    std::unique_ptr<CborConfigurationService<CborUiConfig>> cbor_configuration_service_;
    std::unique_ptr<JsonConfigurationService<JsonUiConfig>> json_configuration_service_;

    std::unique_ptr<UiConfigurationCborParser> cbor_parser_;
    std::unique_ptr<UiConfigurationJsonParser> json_parser_;

    std::shared_ptr<UiConfiguration> configuration_;

    bool ApplyJsonConfiguration(bool fs_load, std::span<const uint8_t> data = {});
    bool CreateDefaultConfiguration();

public:
    explicit UiConfigurationManager(
        std::unique_ptr<CborConfigurationService<CborUiConfig>> cbor_configuration_service,
        std::unique_ptr<JsonConfigurationService<JsonUiConfig>> json_configuration_service);
    bool Update(const UiConfiguration& configuration);
    std::shared_ptr<UiConfiguration> Get(bool force_load = false);

    bool ApplyJsonConfiguration(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::domain::ui_domain::configuration

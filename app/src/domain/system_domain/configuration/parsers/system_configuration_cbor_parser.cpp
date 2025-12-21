#include <utility>

#include "system_configuration_validator.h"
#include "system_configuration_cbor_parser.h"

namespace eerie_leap::domain::system_domain::configuration::parsers {

pmr_unique_ptr<CborSystemConfig> SystemConfigurationCborParser::Serialize(const SystemConfiguration& configuration) {
    SystemConfigurationValidator::Validate(configuration);

    auto config = make_unique_pmr<CborSystemConfig>(Mrm::GetExtPmr());
    memset(config.get(), 0, sizeof(CborSystemConfig));

    config->device_id = configuration.device_id;
    config->hw_version = configuration.hw_version;
    config->sw_version = configuration.sw_version;
    config->build_number = configuration.build_number;
    config->interface_channel = configuration.interface_channel;

    return config;
}

pmr_unique_ptr<SystemConfiguration> SystemConfigurationCborParser::Deserialize(
    std::pmr::memory_resource* mr,
    const CborSystemConfig& config) {

    auto configuration = make_unique_pmr<SystemConfiguration>(mr);

    configuration->device_id = config.device_id;
    configuration->hw_version = config.hw_version;
    configuration->sw_version = config.sw_version;
    configuration->build_number = config.build_number;
    configuration->interface_channel = config.interface_channel;

    SystemConfigurationValidator::Validate(*configuration);

    return configuration;
}

} // namespace eerie_leap::domain::system_domain::configuration::parsers

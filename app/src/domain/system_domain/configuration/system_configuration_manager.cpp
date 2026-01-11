#include "subsys/random/rng.h"

#include "system_configuration_manager.h"

namespace eerie_leap::domain::system_domain::configuration {

using namespace eerie_leap::subsys::random;
using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(system_config_ctrl_logger);

SystemConfigurationManager::SystemConfigurationManager(std::unique_ptr<CborConfigurationService<CborSystemConfig>> cbor_configuration_service) :
    cbor_configuration_service_(std::move(cbor_configuration_service)),
    configuration_(nullptr) {

    cbor_parser_ = std::make_unique<SystemConfigurationCborParser>();
    std::shared_ptr<SystemConfiguration> configuration = nullptr;

    try {
        configuration = Get(true);
    } catch(...) {
        LOG_ERR("Failed to load System configuration.");
    }

    if(configuration == nullptr) {
        if(!CreateDefaultConfiguration()) {
            LOG_ERR("Failed to create default System configuration.");
            return;
        }

        LOG_INF("Default System configuration loaded successfully.");

        configuration = Get();
    }

    UpdateHwVersion(configuration->hw_version);
    UpdateSwVersion(configuration->sw_version);

    LOG_INF("System Configuration Manager initialized successfully.");

    LOG_INF("HW Version: %s, SW Version: %s",
        configuration->GetFormattedHwVersion().c_str(), configuration->GetFormattedSwVersion().c_str());
    LOG_INF("Device ID: %llu", configuration->device_id);
}

bool SystemConfigurationManager::UpdateInterfaceChannel(uint16_t interface_channel) {
    auto configuration = Get();
    if(configuration == nullptr)
        return false;

    if(interface_channel != configuration->interface_channel) {
        configuration->interface_channel = interface_channel;

        bool result = Update(*configuration);
        if(!result)
            return false;

        LOG_INF("Interface channel updated to %u.", configuration->interface_channel);
    }

    return true;
}

bool SystemConfigurationManager::UpdateBuildNumber(uint32_t build_number) {
    auto configuration = Get();
    if(configuration == nullptr)
        return false;

    if(build_number != configuration->build_number) {
        configuration->build_number = build_number;

        bool result = Update(*configuration);
        if(!result)
            return false;

        LOG_INF("Build number updated to %u.", configuration->build_number);
    }

    return true;
}

bool SystemConfigurationManager::UpdateHwVersion(uint32_t hw_version) {
    auto configuration = Get();
    if(configuration == nullptr)
        return false;

    uint32_t config_hw_version = SystemConfiguration::GetConfigHwVersion();
    if(hw_version != config_hw_version) {
        configuration->hw_version = config_hw_version;

        bool result = Update(*configuration);
        if(!result)
            return false;

        LOG_INF("HW version updated to %s.", configuration->GetFormattedHwVersion().c_str());
    }

    return true;
}

bool SystemConfigurationManager::UpdateSwVersion(uint32_t sw_version) {
    auto configuration = Get();
    if(configuration == nullptr)
        return false;

    uint32_t config_sw_version = SystemConfiguration::GetConfigSwVersion();
    if(sw_version != config_sw_version) {
        configuration->sw_version = config_sw_version;
        configuration->build_number = 0;

        bool result = Update(*configuration);
        if(!result)
            return false;

        LOG_INF("SW version updated to %s.", configuration->GetFormattedSwVersion().c_str());
    }

    return true;
}

bool SystemConfigurationManager::Update(const SystemConfiguration& configuration) {
    try {
        auto cbor_config = cbor_parser_->Serialize(configuration);

        if(!cbor_configuration_service_->Save(cbor_config.get()))
            return false;
    } catch(const std::exception& e) {
        LOG_ERR("Failed to update System configuration. %s", e.what());
        return false;
    }

    return Get(true) != nullptr;
}

std::shared_ptr<SystemConfiguration> SystemConfigurationManager::Get(bool force_load) {
    if(configuration_ != nullptr && !force_load)
        return configuration_;

    auto cbor_config_data = cbor_configuration_service_->Load();
    if(!cbor_config_data.has_value())
        return nullptr;

    auto cbor_config = std::move(cbor_config_data.value().config);

    auto configuration = cbor_parser_->Deserialize(Mrm::GetDefaultPmr(), *cbor_config);
    configuration_ = std::make_shared<SystemConfiguration>(std::move(*configuration));

    return configuration_;
}

bool SystemConfigurationManager::CreateDefaultConfiguration() {
    SystemConfiguration system_config = {};
    system_config.device_id = Rng::Get64(true);
    system_config.hw_version = 0;
    system_config.sw_version = 0;
    system_config.build_number = 0;
    system_config.interface_channel = 0;

    return Update(system_config);
}

} // namespace eerie_leap::domain::system_domain::configuration

#pragma once

#include <atomic>
#include <memory>

#include <zephyr/kernel.h>

#include "domain/configuration_domain/services/configuration_service.h"

namespace eerie_leap::domain::ble_settings_domain::services {

using namespace eerie_leap::domain::configuration_domain::services;

class BleSettingsConfigurationService {
private:
    static std::shared_ptr<ConfigurationService> configuration_service_;

    BleSettingsConfigurationService() = default;
    ~BleSettingsConfigurationService() = default;

    BleSettingsConfigurationService(const BleSettingsConfigurationService&) = delete;
    BleSettingsConfigurationService& operator=(const BleSettingsConfigurationService&) = delete;

    static bool HandleConfigWrite(uint8_t settings_id, std::span<const uint8_t> data);
    static std::span<const uint8_t> HandleConfigRead(uint8_t settings_id);

public:
    static int Initialize(std::shared_ptr<ConfigurationService> configuration_service);
};

} // namespace eerie_leap::domain::ble_settings_domain::services

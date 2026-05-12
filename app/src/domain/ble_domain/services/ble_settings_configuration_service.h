#pragma once

#include <memory>

#include <zephyr/kernel.h>

#include "domain/configuration_domain/services/configuration_service.h"

namespace eerie_leap::domain::ble_domain::services {

using eerie_leap::domain::configuration_domain::services::ConfigurationService;

class BleSettingsConfigurationService {
private:
    static std::unique_ptr<BleSettingsConfigurationService> instance_;
    static bool is_initialized_;
    static std::pmr::string json_str_buffer_;

    std::shared_ptr<ConfigurationService> configuration_service_;

    explicit BleSettingsConfigurationService(std::shared_ptr<ConfigurationService> configuration_service);

    bool HandleConfigWrite(uint8_t settings_id, std::span<const uint8_t> data) const;
    std::span<const uint8_t> HandleConfigRead(uint8_t settings_id) const;

public:
    BleSettingsConfigurationService& operator=(const BleSettingsConfigurationService&) = delete;

    static BleSettingsConfigurationService& Create(std::shared_ptr<ConfigurationService> configuration_service);
    static BleSettingsConfigurationService& GetInstance();

    bool Initialize();
};

} // namespace eerie_leap::domain::ble_domain::services

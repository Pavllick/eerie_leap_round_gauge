#include <exception>

#include <zephyr/logging/log.h>

#include "utilities/memory/memory_resource_manager.h"
#include "subsys/bluetooth/ble.h"
#include "subsys/bluetooth/ble_settings/ble_settings_service.h"

#include "ble_settings_configuration_service.h"

namespace eerie_leap::domain::ble_domain::services {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::subsys::bluetooth;
using namespace eerie_leap::subsys::bluetooth::ble_settings;

LOG_MODULE_REGISTER(ble_scs_logger);

std::unique_ptr<BleSettingsConfigurationService> BleSettingsConfigurationService::instance_;
bool BleSettingsConfigurationService::is_initialized_ = false;
std::pmr::string BleSettingsConfigurationService::json_str_buffer_ = "";

BleSettingsConfigurationService::BleSettingsConfigurationService(std::shared_ptr<ConfigurationService> configuration_service)
    : configuration_service_(std::move(configuration_service)) {}

BleSettingsConfigurationService& BleSettingsConfigurationService::Create(
    std::shared_ptr<ConfigurationService> configuration_service) {

    instance_.reset(new BleSettingsConfigurationService(std::move(configuration_service)));

    return *instance_;
}

BleSettingsConfigurationService& BleSettingsConfigurationService::GetInstance() {
    return *instance_;
}

bool BleSettingsConfigurationService::Initialize() {
    if (is_initialized_)
        return true;

    is_initialized_ = true;

    Ble::RegisterConnectedHandler(BleSettingsService::BleConnected);
    Ble::RegisterDisconnectedHandler(BleSettingsService::BleDisconnected);

    BleSettingsService::Initialize({
            .on_config_write = [this](uint8_t settings_id, std::span<const uint8_t> data) {
                return HandleConfigWrite(settings_id, data);
            },
            .on_config_read = [this](uint8_t settings_id) {
                return HandleConfigRead(settings_id);
            },
        },
        Mrm::GetExtPmr(),
        64 * 1024);

    return true;
}

bool BleSettingsConfigurationService::HandleConfigWrite(uint8_t settings_id, std::span<const uint8_t> data) const {
    auto type = static_cast<ConfigurationService::Type>(settings_id);
    std::string_view json_str(reinterpret_cast<const char*>(data.data()), data.size());

    try {
        return configuration_service_->ApplyJsonConfiguration(type, json_str);
    } catch (...) {
        LOG_ERR("Failed to apply JSON configuration for settings_id=%d", settings_id);
        return false;
    }
}

std::span<const uint8_t> BleSettingsConfigurationService::HandleConfigRead(uint8_t settings_id) const {
    auto type = static_cast<ConfigurationService::Type>(settings_id);

    try {
        json_str_buffer_ = configuration_service_->GetJsonConfiguration(type);
    } catch (const std::exception& e) {
        LOG_ERR("Failed to get JSON configuration for settings_id=%d: %s", settings_id, e.what());
        json_str_buffer_.clear();
    } catch (...) {
        LOG_ERR("Failed to get JSON configuration for settings_id=%d", settings_id);
        json_str_buffer_.clear();
    }

    return { reinterpret_cast<const uint8_t*>(json_str_buffer_.c_str()), json_str_buffer_.size() };
}

} // namespace eerie_leap::domain::ble_domain::services

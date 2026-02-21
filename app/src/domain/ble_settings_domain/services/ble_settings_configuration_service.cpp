#include <zephyr/logging/log.h>

#include "utilities/memory/memory_resource_manager.h"
#include "subsys/bluetooth/ble.h"
#include "subsys/bluetooth/ble_settings/ble_settings_service.h"

#include "ble_settings_configuration_service.h"

namespace eerie_leap::domain::ble_settings_domain::services {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::subsys::bluetooth;
using namespace eerie_leap::subsys::bluetooth::ble_settings;

LOG_MODULE_REGISTER(ble_scs_logger);

std::shared_ptr<ConfigurationService> BleSettingsConfigurationService::configuration_service_ = nullptr;

int BleSettingsConfigurationService::Initialize(std::shared_ptr<ConfigurationService> configuration_service) {
    configuration_service_ = std::move(configuration_service);

    Ble::RegisterConnectedHandler(BleSettingsService::BleConnected);
    Ble::RegisterDisconnectedHandler(BleSettingsService::BleDisconnected);

    BleSettingsService::Initialize({
            .on_config_write = HandleConfigWrite,
            .on_config_read = HandleConfigRead,
        },
        Mrm::GetExtPmr(),
        64 * 1024);

    return 0;
}

bool BleSettingsConfigurationService::HandleConfigWrite(uint8_t settings_id, std::span<const uint8_t> data) {
    auto type = static_cast<ConfigurationService::Type>(settings_id);
    return configuration_service_->ApplyJsonConfiguration(type, data);
}

std::string test = "test";
std::span<const uint8_t> BleSettingsConfigurationService::HandleConfigRead(uint8_t settings_id) {
    auto type = static_cast<ConfigurationService::Type>(settings_id);
    // return configuration_service_->GetJsonConfiguration(type);

    switch(type) {
        case ConfigurationService::Type::CanbusJson:
            // return config data
            return { reinterpret_cast<const uint8_t*>(test.c_str()), test.size() };

        default:
            return {};
    }
}

} // namespace eerie_leap::domain::ble_settings_domain::services

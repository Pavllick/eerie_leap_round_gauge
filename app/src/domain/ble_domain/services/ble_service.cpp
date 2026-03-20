#include <zephyr/bluetooth/bluetooth.h>

#include "subsys/bluetooth/ble.h"
#include "subsys/bluetooth/utilities/bt_data_builder.hpp"
#include "subsys/bluetooth/ble_settings/ble_settings_service.h"
#include "domain/ble_domain/services/ble_settings_configuration_service.h"

#include "ble_service.h"

namespace eerie_leap::domain::ble_domain::services {

using namespace eerie_leap::subsys::bluetooth;
using namespace eerie_leap::subsys::bluetooth::utilities;
using namespace eerie_leap::subsys::bluetooth::ble_settings;

std::unique_ptr<BleService> BleService::instance_;
bool BleService::is_initialized_ = false;

BleService::BleService(
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SensorsProcessingService> sensors_processing_service)
        : configuration_service_(std::move(configuration_service)),
        sensors_processing_service_(std::move(sensors_processing_service)) {}

BleService& BleService::Create(
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SensorsProcessingService> sensors_processing_service) {

    instance_.reset(new BleService(
        configuration_service, std::move(sensors_processing_service)));

    BleSettingsConfigurationService::Create(configuration_service);

    return *instance_;
}

BleService& BleService::GetInstance() {
    return *instance_;
}

bool BleService::Initialize() {
    if(is_initialized_)
        return true;

    is_initialized_ = true;

    Ble::Initialize();
    ConfigureAdvertisingData();
    ConfigureScanResponseData();

    Ble::RegisterPairingStartedHandler([this]() {
        PairingStarted();
    });
    Ble::RegisterPairingFinishedHandler([this]() {
        PairingFinished();
    });

    auto& settings_service = BleSettingsConfigurationService::GetInstance();
    if(!settings_service.Initialize())
        return false;

    return true;
}

bool BleService::Start() {
    return Ble::Start();
}

void BleService::ConfigureAdvertisingData() {
    BtDataBuilder ad_builder;
    // Flags: general discoverable, no BR/EDR
    const uint8_t flags[] = {BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR};
    ad_builder.Add(BT_DATA_FLAGS, flags, sizeof(flags));
    // Full device name
    ad_builder.Add(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1);

    Ble::UpdateAdvertisingData(ad_builder.Build());
}

void BleService::ConfigureScanResponseData() {
    BtDataBuilder sd_builder;

    const uint8_t config_service_uuid[] = { BT_UUID_SETTINGS_SERVICE_VAL };
    sd_builder.Add(BT_DATA_UUID128_ALL, config_service_uuid, sizeof(config_service_uuid));

    Ble::UpdateScanResponseData(sd_builder.Build());
}

void BleService::PairingStarted() {
    sensors_processing_service_->Pause();
}

void BleService::PairingFinished() {
    sensors_processing_service_->Resume();
}

} // namespace eerie_leap::domain::ble_domain::services

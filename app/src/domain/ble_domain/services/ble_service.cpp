#include <vector>
#include <utility>

#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>

#include "subsys/bluetooth/ble.h"
#include "subsys/bluetooth/utilities/bt_data_builder.hpp"
#include "subsys/bluetooth/ble_settings/ble_settings_service.h"
#include "domain/ble_domain/services/ble_settings_configuration_service.h"
#include "domain/system_domain/models/product_info.h"

#include "ble_service.h"

namespace eerie_leap::domain::ble_domain::services {

using namespace eerie_leap::subsys::bluetooth;
using namespace eerie_leap::subsys::bluetooth::utilities;
using namespace eerie_leap::subsys::bluetooth::ble_settings;
using namespace eerie_leap::domain::system_domain::models;

std::unique_ptr<BleService> BleService::instance_;
bool BleService::is_initialized_ = false;

BleService::BleService(std::shared_ptr<SensorsProcessingService> sensors_processing_service)
    : sensors_processing_service_(std::move(sensors_processing_service)) {}

BleService& BleService::Create(
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SensorsProcessingService> sensors_processing_service) {

    BleSettingsConfigurationService::Create(std::move(configuration_service));
    instance_.reset(new BleService(std::move(sensors_processing_service)));

    return *instance_;
}

BleService& BleService::GetInstance() {
    if(!instance_)
        throw std::runtime_error("BleService instance not created. Call Create() first.");

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

    return BleSettingsConfigurationService::GetInstance().Initialize();
}

bool BleService::Start() const {
    return Ble::Start();
}

void BleService::ConfigureAdvertisingData() const {
    BtDataBuilder ad_builder;
    // Flags: general discoverable, no BR/EDR
    const std::vector<uint8_t> flags = { BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };
    ad_builder.Add(BT_DATA_FLAGS, flags);
    // Full device name
    ad_builder.Add(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1);

    // Manufacturer data
    auto manufacturer_data = GetManufacturerData();
    ad_builder.Add(BT_DATA_MANUFACTURER_DATA, manufacturer_data);

    Ble::UpdateAdvertisingData(ad_builder.Build());
}

void BleService::ConfigureScanResponseData() const {
    BtDataBuilder sd_builder;

    const std::vector<uint8_t> config_service_uuid = { BT_UUID_SETTINGS_SERVICE_VAL };
    sd_builder.Add(BT_DATA_UUID128_ALL, config_service_uuid);

    Ble::UpdateScanResponseData(sd_builder.Build());
}

void BleService::PairingStarted() const {
    if(sensors_processing_service_)
        sensors_processing_service_->Pause();
}

void BleService::PairingFinished() const {
    if(sensors_processing_service_)
        sensors_processing_service_->Resume();
}

std::vector<uint8_t> BleService::GetManufacturerData() {
    // 2 (SIG company ID)
    // 1 (family)
    // 2 (product ID)
    // 2 (product features)
    // 1 (revision)
    std::vector<uint8_t> manufacturer_data;
    manufacturer_data.reserve(8);

    // Bluetooth SIG Company ID - must be first 2 bytes, little-endian
    // 0xFFFF = reserved for internal use / testing
    constexpr uint16_t company_id = 0xFFFF;
    const uint16_t company_id_le = sys_cpu_to_le16(company_id);
    const auto* cid = reinterpret_cast<const uint8_t*>(&company_id_le);
    manufacturer_data.insert(manufacturer_data.end(), cid, cid + sizeof(company_id_le));

    // Product family (1 byte)
    constexpr uint8_t product_family_le = std::to_underlying(ProductInfo::family);
    const auto* product_family_ptr = &product_family_le;
    manufacturer_data.insert(manufacturer_data.end(), product_family_ptr, product_family_ptr + sizeof(product_family_le));

    // Product ID (2 bytes, little-endian)
    constexpr uint16_t product_id_le = sys_cpu_to_le16(ProductInfo::product_id);
    const auto* product_id_ptr = reinterpret_cast<const uint8_t*>(&product_id_le);
    manufacturer_data.insert(manufacturer_data.end(), product_id_ptr, product_id_ptr + sizeof(product_id_le));

    // Product features (2 bytes, little-endian)
    constexpr uint16_t product_features_le = sys_cpu_to_le16(ProductInfo::features);
    const auto* product_features_ptr = reinterpret_cast<const uint8_t*>(&product_features_le);
    manufacturer_data.insert(manufacturer_data.end(), product_features_ptr, product_features_ptr + sizeof(product_features_le));

    // Product revision (1 byte)
    constexpr uint8_t product_revision_le = ProductInfo::revision;
    const auto* product_revision_ptr = &product_revision_le;
    manufacturer_data.insert(manufacturer_data.end(), product_revision_ptr, product_revision_ptr + sizeof(product_revision_le));

    return manufacturer_data;
}

} // namespace eerie_leap::domain::ble_domain::services

#pragma once

#include <memory>

#include "domain/configuration_domain/services/configuration_service.h"
#include "domain/sensor_domain/services/sensors_processing_service.h"

namespace eerie_leap::domain::ble_domain::services {

using eerie_leap::domain::configuration_domain::services::ConfigurationService;
using eerie_leap::domain::sensor_domain::services::SensorsProcessingService;

class BleService {
private:
    static std::unique_ptr<BleService> instance_;

    std::shared_ptr<SensorsProcessingService> sensors_processing_service_;

    static bool is_initialized_;

    static std::vector<uint8_t> GetManufacturerData();

    void ConfigureAdvertisingData() const;
    void ConfigureScanResponseData() const;
    void PairingStarted() const;
    void PairingFinished() const;

    BleService(std::shared_ptr<SensorsProcessingService> sensors_processing_service);

public:
    BleService& operator=(const BleService&) = delete;

    static BleService& Create(
        std::shared_ptr<ConfigurationService> configuration_service,
        std::shared_ptr<SensorsProcessingService> sensors_processing_service);
    static BleService& GetInstance();

    bool Initialize();
    bool Start() const;
};

} // namespace eerie_leap::domain::ble_domain::services

#pragma once

#include <memory>

#include "domain/configuration_domain/services/configuration_service.h"
#include "domain/sensor_domain/services/sensors_processing_service.h"

namespace eerie_leap::domain::ble_domain::services {

using namespace eerie_leap::domain::configuration_domain::services;
using namespace eerie_leap::domain::sensor_domain::services;

class BleService {
private:
    static std::unique_ptr<BleService> instance_;

    std::shared_ptr<ConfigurationService> configuration_service_;
    std::shared_ptr<SensorsProcessingService> sensors_processing_service_;

    static bool is_initialized_;

    void ConfigureAdvertisingData();
    void ConfigureScanResponseData();
    void PairingStarted();
    void PairingFinished();

    BleService(
        std::shared_ptr<ConfigurationService> configuration_service,
        std::shared_ptr<SensorsProcessingService> sensors_processing_service);

public:
    BleService& operator=(const BleService&) = delete;

    static BleService& Create(
        std::shared_ptr<ConfigurationService> configuration_service,
        std::shared_ptr<SensorsProcessingService> sensors_processing_service);
    static BleService& GetInstance();

    bool Initialize();
    bool Start();
};

} // namespace eerie_leap::domain::ble_domain::services

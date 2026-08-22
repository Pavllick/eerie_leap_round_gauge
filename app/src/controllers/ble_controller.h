#pragma once

#include <memory>

#include "domain/ble_domain/services/ble_service.h"
#include "domain/configuration_domain/services/configuration_service.h"
#include "domain/sensor_domain/services/sensors_processing_service.h"

namespace eerie_leap::controllers {

using eerie_leap::domain::ble_domain::services::BleService;
using eerie_leap::domain::configuration_domain::services::ConfigurationService;
using eerie_leap::domain::sensor_domain::services::SensorsProcessingService;

class BleController {
private:
    std::shared_ptr<ConfigurationService> configuration_service_;
    std::shared_ptr<SensorsProcessingService> sensors_processing_service_;

    BleService* ble_service_ = nullptr;

public:
    BleController(
        std::shared_ptr<ConfigurationService> configuration_service,
        std::shared_ptr<SensorsProcessingService> sensors_processing_service);

    int Initialize();
    int Start();
};

} // namespace eerie_leap::controllers

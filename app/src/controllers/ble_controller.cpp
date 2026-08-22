#include <zephyr/logging/log.h>

#include "ble_controller.h"

namespace eerie_leap::controllers {

LOG_MODULE_REGISTER(ble_controller_logger);

BleController::BleController(
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SensorsProcessingService> sensors_processing_service)
    : configuration_service_(std::move(configuration_service)),
      sensors_processing_service_(std::move(sensors_processing_service)) {}

int BleController::Initialize() {
    ble_service_ = &BleService::Create(configuration_service_, sensors_processing_service_);

    if(!ble_service_->Initialize()) {
        LOG_ERR("Failed to initialize the BLE service.");
        return -1;
    }

    return 0;
}

int BleController::Start() {
    return ble_service_->Start() ? 0 : -1;
}

} // namespace eerie_leap::controllers

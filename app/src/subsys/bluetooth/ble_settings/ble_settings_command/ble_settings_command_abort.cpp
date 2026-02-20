#include "ble_settings_command_abort.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandAbort::BleSettingsCommandAbort(std::shared_ptr<BleSettingsStatus> status)
    : BleSettingsCommandRequestBase(status) {}

void BleSettingsCommandAbort::Process(std::span<const uint8_t> data) {
    status_->Reset();
    LOG_INF("Abort command processed");
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

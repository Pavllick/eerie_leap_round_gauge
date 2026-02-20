#include "ble_settings_command_start_read.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandStartRead::BleSettingsCommandStartRead(std::shared_ptr<BleSettingsStatus> status)
    : BleSettingsCommandBase(status) {}

void BleSettingsCommandStartRead::Process(std::span<const uint8_t> data) {
    // TODO: Implement start read logic

    LOG_INF("Start read command processed");
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

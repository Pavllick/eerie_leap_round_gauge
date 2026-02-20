#include "ble_settings_command_end_read.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandEndRead::BleSettingsCommandEndRead(std::shared_ptr<BleSettingsStatus> status)
    : BleSettingsCommandBase(status) {}

void BleSettingsCommandEndRead::Process(std::span<const uint8_t> data) {
    // TODO: Implement end read logic

    LOG_INF("End read command processed");
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

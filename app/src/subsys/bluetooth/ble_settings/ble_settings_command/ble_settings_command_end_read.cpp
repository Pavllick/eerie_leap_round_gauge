#include "ble_settings_command_end_read.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

std::vector<uint8_t> BleSettingsCommandEndRead::Create() {
    return {
        static_cast<uint8_t>(BleSettingsCommandType::EndRead)
    };
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

#include "ble_settings_command_start_read.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

std::vector<uint8_t> BleSettingsCommandStartRead::Create(uint8_t settings_id, uint32_t size) {
    return {
        static_cast<uint8_t>(BleSettingsCommandType::StartRead),
        settings_id,
        static_cast<uint8_t>(size & 0xFF),
        static_cast<uint8_t>((size >> 8) & 0xFF),
        static_cast<uint8_t>((size >> 16) & 0xFF),
        static_cast<uint8_t>((size >> 24) & 0xFF)
    };
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

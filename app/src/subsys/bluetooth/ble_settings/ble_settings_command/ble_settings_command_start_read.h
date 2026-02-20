#pragma once

#include <cstdint>
#include <vector>

#include "../ble_settings_service_enums.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

class BleSettingsCommandStartRead {
public:
    static std::vector<uint8_t> Create(uint8_t settings_id, uint32_t size);
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

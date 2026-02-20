#pragma once

#include <cstdint>
#include <vector>

#include "../ble_settings_service_enums.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

class BleSettingsCommandEndRead {
public:
    static std::vector<uint8_t> Create();
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

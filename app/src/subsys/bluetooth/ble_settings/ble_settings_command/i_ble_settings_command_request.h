#pragma once

#include <cstdint>
#include <span>

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

class IBleSettingsCommandRequest {
public:
    virtual ~IBleSettingsCommandRequest() = default;
    virtual void Process(std::span<const uint8_t> data) = 0;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

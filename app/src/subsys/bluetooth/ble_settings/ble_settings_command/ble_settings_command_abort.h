#pragma once

#include <cstdint>
#include <span>
#include <memory>

#include "ble_settings_command_base.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

// NOTE: Data format:
//       [0] - BleSettingsCommandType::Abort
class BleSettingsCommandAbort : public BleSettingsCommandBase {
public:
    explicit BleSettingsCommandAbort(std::shared_ptr<BleSettingsStatus> status);
    virtual ~BleSettingsCommandAbort() = default;

    void Process(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

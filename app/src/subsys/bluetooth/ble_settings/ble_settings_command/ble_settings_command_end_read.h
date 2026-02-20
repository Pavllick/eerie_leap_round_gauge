#pragma once

#include <cstdint>
#include <span>
#include <memory>

#include "ble_settings_command_base.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

// NOTE: Data format:
//       [0] - BleSettingsCommandType::EndRead
class BleSettingsCommandEndRead : public BleSettingsCommandBase {
public:
    explicit BleSettingsCommandEndRead(std::shared_ptr<BleSettingsStatus> status);
    virtual ~BleSettingsCommandEndRead() = default;

    void Process(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

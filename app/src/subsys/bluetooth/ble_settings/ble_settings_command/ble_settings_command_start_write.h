#pragma once

#include <cstdint>
#include <span>
#include <memory>

#include "ble_settings_command_base.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

// NOTE: Data format:
//       [0] - BleSettingsCommandType::StartWrite
//       [1] - BleSettingsType
//       [2-5] - data size, uint32_t (little-endian)
class BleSettingsCommandStartWrite : public BleSettingsCommandBase {
private:
    size_t max_transfer_size_{0};

public:
    explicit BleSettingsCommandStartWrite(std::shared_ptr<BleSettingsStatus> status);
    virtual ~BleSettingsCommandStartWrite() = default;

    void SetMaxTransferSize(size_t max_transfer_size);
    void Process(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <memory_resource>
#include <functional>

#include "ble_settings_command_base.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

// NOTE: Data format:
//       [0] - BleSettingsCommandType::EndWrite
class BleSettingsCommandEndWrite : public BleSettingsCommandBase {
public:
    using WriteHandler = std::function<bool(BleSettingsType type, std::span<const uint8_t> data)>;

private:
    std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer_;
    WriteHandler write_handler_;

public:
    explicit BleSettingsCommandEndWrite(
        std::shared_ptr<BleSettingsStatus> status,
        std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer);
    virtual ~BleSettingsCommandEndWrite() = default;

    void SetWriteHandler(WriteHandler write_handler);
    void Process(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

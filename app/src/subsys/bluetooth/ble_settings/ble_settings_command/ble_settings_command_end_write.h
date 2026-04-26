#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <memory_resource>
#include <functional>

#include "ble_settings_command_request_base.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

// NOTE: Data format:
//       [0] - BleSettingsCommandType::EndWrite
class BleSettingsCommandEndWrite : public BleSettingsCommandRequestBase {
public:
    using WriteHandler = std::function<bool(uint8_t settings_id, std::span<const uint8_t> data)>;

private:
    std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer_;
    WriteHandler write_handler_;

public:
    explicit BleSettingsCommandEndWrite(
        std::shared_ptr<BleSettingsStatus> status);
    virtual ~BleSettingsCommandEndWrite() = default;

    void Initialize(std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer, const WriteHandler& write_handler);

    void Process(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

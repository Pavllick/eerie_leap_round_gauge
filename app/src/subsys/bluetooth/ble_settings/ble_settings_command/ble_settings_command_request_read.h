#pragma once

#include <cstdint>
#include <span>
#include <memory>

#include "ble_settings_command_base.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

// NOTE: Data format:
//       [0] - BleSettingsCommandType::RequestRead
//       [1] - BleSettingsType
class BleSettingsCommandRequestRead : public BleSettingsCommandBase {
public:
    using ReadHandler = std::function<std::span<const uint8_t>(BleSettingsType)>;
    using SendHandler = std::function<bool(BleSettingsType, std::span<const uint8_t>)>;

private:
    ReadHandler read_handler_;
    SendHandler send_handler_;

public:
    explicit BleSettingsCommandRequestRead(std::shared_ptr<BleSettingsStatus> status);
    virtual ~BleSettingsCommandRequestRead() = default;

    void SetReadHandler(ReadHandler handler);
    void SetSendHandler(SendHandler send_handler);

    void Process(std::span<const uint8_t> data) override;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command

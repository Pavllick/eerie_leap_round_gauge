#pragma once

#include <cstdint>

namespace eerie_leap::subsys::bluetooth::ble_settings {

enum class BleSettingsCommandType : uint8_t {
    Abort = 0x01,

    // Request commands
    StartWrite = 0x20,
    EndWrite = 0x21,
    RequestRead = 0x22,

    // Notification commands
    StartRead = 0x40,
    EndRead = 0x41,
};

enum class BleSettingsState : uint8_t {
    Idle = 0,
    Writing = 1,
    Reading = 2,
    Error = 3
};

enum class BleSettingsErrorCode : uint8_t {
    None = 0,
    InsufficientData = 1,    // "StartWrite" command too short
    TransferTooLarge = 2,    // Requested size exceeds buffer
    InvalidState = 3,        // Command received in wrong state
    IncompleteTransfer = 4,  // "EndWrite" before all data received
    HandlerFailed = 5,       // on_config_write callback returned false
    DataOverflow = 6,        // Received more data than expected
    NotificationFailed = 7,  // Failed to send BLE notification
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings

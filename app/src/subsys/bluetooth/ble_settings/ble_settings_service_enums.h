#pragma once

#include <cstdint>

namespace eerie_leap::subsys::bluetooth::ble_settings {

enum class BleSettingsType : uint8_t {
    CanBus = 0x01,
    WifiSettings = 0x02,
    SensorCalibration = 0x03,
    SystemParams = 0x04,
};

enum class BleSettingsCommandType : uint8_t {
    StartWrite = 0x01,
    EndWrite = 0x02,
    RequestRead = 0x03,
    StartRead = 0x04,
    EndRead = 0x05,
    Abort = 0x06,
};

enum class BleSettingsState : uint8_t {
    Idle = 0,
    Writing = 1,
    Reading = 2,
    Error = 3
};

enum class BleSettingsErrorCode : uint8_t {
    None = 0,
    InsufficientData = 1,    // START_WRITE command too short
    TransferTooLarge = 2,    // Requested size exceeds buffer
    InvalidState = 3,        // Command received in wrong state
    IncompleteTransfer = 4,  // END_WRITE before all data received
    HandlerFailed = 5,       // on_config_write callback returned false
    DataOverflow = 6,        // Received more data than expected
    NotificationFailed = 7,  // Failed to send BLE notification
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings

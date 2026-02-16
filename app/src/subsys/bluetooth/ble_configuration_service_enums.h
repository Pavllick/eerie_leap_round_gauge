#pragma once

#include <span>
#include <functional>
#include <cstdint>
#include <optional>
#include <vector>
#include <memory_resource>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/kernel.h>

namespace eerie_leap::subsys::bluetooth {

enum class BleConfigServiceType : uint8_t {
    CanBus = 0x01,
    WifiSettings = 0x02,
    SensorCalibration = 0x03,
    SystemParams = 0x04,
};

enum class BleConfigServiceCommand : uint8_t {
    StartWrite = 0x01,
    EndWrite = 0x02,
    Abort = 0x03,
    RequestRead = 0x04,
    StartRead = 0x05,
    ReadComplete = 0x06,
};

enum class BleConfigServiceState : uint8_t {
    Idle = 0,
    Writing = 1,
    Reading = 2,
    Error = 3
};

enum class BleConfigServiceErrorCode : uint8_t {
    None = 0,
    InsufficientData = 1,    // START_WRITE command too short
    TransferTooLarge = 2,    // Requested size exceeds buffer
    InvalidState = 3,        // Command received in wrong state
    IncompleteTransfer = 4,  // END_WRITE before all data received
    HandlerFailed = 5,       // on_config_write callback returned false
    DataOverflow = 6,        // Received more data than expected
    NotificationFailed = 7,  // Failed to send BLE notification
};

} // namespace eerie_leap::subsys::bluetooth

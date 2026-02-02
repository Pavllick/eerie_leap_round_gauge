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

enum class ConfigType : uint8_t {
    CanBus = 0x01,
    WifiSettings = 0x02,
    SensorCalibration = 0x03,
    SystemParams = 0x04,
};

enum class Command : uint8_t {
    StartWrite = 0x01,
    EndWrite = 0x02,
    Abort = 0x03,
    RequestRead = 0x04,
    StartRead = 0x05,
    ReadComplete = 0x06,
};

enum class State : uint8_t {
    Idle = 0,
    Writing = 1,
    Reading = 2,
    Error = 3
};

enum class ErrorCode : uint8_t {
    None = 0,
    InsufficientData = 1,    // START_WRITE command too short
    TransferTooLarge = 2,    // Requested size exceeds buffer
    InvalidState = 3,        // Command received in wrong state
    IncompleteTransfer = 4,  // END_WRITE before all data received
    HandlerFailed = 5,       // on_config_write callback returned false
    DataOverflow = 6,        // Received more data than expected
    NotificationFailed = 7,  // Failed to send BLE notification
};

struct Status {
    State state;
    ConfigType current_type;
    uint32_t transferred_bytes;
    uint32_t total_bytes;
    ErrorCode error_code;
} __attribute__((packed));

struct Callbacks {
    using WriteHandler = std::function<bool(ConfigType type, std::span<const uint8_t> data)>;
    using ReadHandler = std::function<size_t(ConfigType type, std::span<uint8_t> buffer)>;
    using StateChangeHandler = std::function<void(State old_state, State new_state)>;

    WriteHandler on_config_write;
    ReadHandler on_config_read;
    StateChangeHandler on_state_change;
};

class BluetoothConfigurationService {
public:
    using allocator_type = std::pmr::polymorphic_allocator<>;

    static constexpr size_t DefaultMaxTransferSize = 64 * 1024;
    static constexpr uint32_t ChunkDelayMs = 5;

private:
    size_t max_transfer_size_{0};

    k_mutex mutex_;
    Callbacks callbacks_;
    Status status_{};
    std::optional<std::pmr::vector<uint8_t>> transfer_buffer_;
    bt_conn* active_conn_{nullptr};
    atomic_t notifications_enabled_{0};
    atomic_t disconnected_during_read_{0};

    bt_le_ext_adv* extended_advertising_{nullptr};
    k_work_delayable adv_restart_work_;

    BluetoothConfigurationService() = default;
    ~BluetoothConfigurationService() = default;

    BluetoothConfigurationService(const BluetoothConfigurationService&) = delete;
    BluetoothConfigurationService& operator=(const BluetoothConfigurationService&) = delete;

    // Must be called with mutex_ held.
    void SetState(State new_state);

    // Must be called with mutex_ held. Unrefs active_conn_ if set.
    void ResetTransferLocked();

    // Acquires mutex_ internally.
    void HandleControlCommand(bt_conn* conn, std::span<const uint8_t> data);

    // Acquires mutex_ internally.
    void HandleDataChunk(std::span<const uint8_t> data);

    int StartAdvertising(k_timeout_t delay);
    int StartExtendedAdvertising();
    int InitializeBluetooth();

    friend ssize_t ControlWriteCallback(
        bt_conn* conn,
        const bt_gatt_attr* attr,
        const void* buf,
        uint16_t len,
        uint16_t offset,
        uint8_t flags);

    friend ssize_t DataWriteCallback(
        bt_conn* conn,
        const bt_gatt_attr* attr,
        const void* buf,
        uint16_t len,
        uint16_t offset,
        uint8_t flags);

    friend ssize_t StatusReadCallback(
        bt_conn* conn,
        const bt_gatt_attr* attr,
        void* buf,
        uint16_t len,
        uint16_t offset);

    friend void NotifyCccChanged(const bt_gatt_attr* attr, uint16_t value);
    friend void Connected(bt_conn* conn, uint8_t err);
    friend void Disconnected(bt_conn* conn, uint8_t reason);

    friend void AdvRestartWorkHandler(struct k_work* work);

public:
    static BluetoothConfigurationService& GetInstance();

    bool Initialize(
        const Callbacks& callbacks,
        allocator_type allocator = std::pmr::get_default_resource(),
        size_t max_transfer_size = DefaultMaxTransferSize);

    [[nodiscard]] Status GetStatus();
    [[nodiscard]] size_t GetMaxTransferSize() const { return max_transfer_size_; }

    bool SendConfig(
        bt_conn* conn,
        ConfigType type);
};

} // namespace eerie_leap::subsys::bluetooth

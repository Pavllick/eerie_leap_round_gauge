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

#include "ble_configuration_service_enums.h"

namespace eerie_leap::subsys::bluetooth {

struct BleConfigurationServiceStatus {
    BleConfigServiceState state;
    BleConfigServiceType current_type;
    uint32_t transferred_bytes;
    uint32_t total_bytes;
    BleConfigServiceErrorCode error_code;
} __attribute__((packed));

struct BleConfigurationServiceCallbacks {
    using WriteHandler = std::function<bool(BleConfigServiceType type, std::span<const uint8_t> data)>;
    using ReadHandler = std::function<size_t(BleConfigServiceType type, std::span<uint8_t> buffer)>;
    using StateChangeHandler = std::function<void(BleConfigServiceState old_state, BleConfigServiceState new_state)>;

    WriteHandler on_config_write;
    ReadHandler on_config_read;
    StateChangeHandler on_state_change;
};

class BleConfigurationService {
public:
    using allocator_type = std::pmr::polymorphic_allocator<>;

    static constexpr size_t DefaultMaxTransferSize = 64 * 1024;
    static constexpr uint32_t ChunkDelayMs = 5;

private:
    static size_t max_transfer_size_;
    static const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, gatt_service_);

    static k_mutex mutex_;
    static BleConfigurationServiceCallbacks callbacks_;
    static BleConfigurationServiceStatus status_;
    static std::optional<std::pmr::vector<uint8_t>> transfer_buffer_;
    static atomic_t disconnected_during_read_;

    BleConfigurationService() = default;
    ~BleConfigurationService() = default;

    BleConfigurationService(const BleConfigurationService&) = delete;
    BleConfigurationService& operator=(const BleConfigurationService&) = delete;

    static void SetState(BleConfigServiceState new_state);
    static void ResetTransfer();
    static void HandleControlCommand(bt_conn* conn, std::span<const uint8_t> data);
    static void HandleDataChunk(std::span<const uint8_t> data);

    static void CommandStartWrite(bt_conn* conn, std::span<const uint8_t> data);
    static void CommandEndWrite(bt_conn* conn, std::span<const uint8_t> data);
    static void CommandAbort(bt_conn* conn, std::span<const uint8_t> data);
    static void CommandRequestRead(bt_conn* conn, std::span<const uint8_t> data);
    static void CommandStartRead(bt_conn* conn, std::span<const uint8_t> data);
    static void CommandReadComplete(bt_conn* conn, std::span<const uint8_t> data);

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

public:
    static void Initialize(
        const BleConfigurationServiceCallbacks& callbacks,
        allocator_type allocator = std::pmr::get_default_resource(),
        size_t max_transfer_size = DefaultMaxTransferSize);

    static void BleConnected(bt_conn* conn);
    static void BleDisconnected(bt_conn* conn);

    [[nodiscard]] static BleConfigurationServiceStatus GetStatus();
    [[nodiscard]] static size_t GetMaxTransferSize() { return max_transfer_size_; }

    static bool SendConfig(
        bt_conn* conn,
        BleConfigServiceType type);
};

} // namespace eerie_leap::subsys::bluetooth

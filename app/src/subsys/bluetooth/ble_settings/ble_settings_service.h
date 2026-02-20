#pragma once

#include <span>
#include <functional>
#include <cstdint>
#include <optional>
#include <vector>
#include <memory_resource>
#include <memory>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/kernel.h>

#include "ble_settings_command/ble_settings_command_manager.h"
#include "ble_settings_status.h"

namespace eerie_leap::subsys::bluetooth::ble_settings {

using namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command;

class BleSettingsService {
public:
    using allocator_type = std::pmr::polymorphic_allocator<>;

    struct Callbacks {
        BleSettingsCommandEndWrite::WriteHandler on_config_write;
        BleSettingsCommandRequestRead::ReadHandler on_config_read;
        BleSettingsStatus::StateChangeHandler on_state_change;
    };

    static constexpr size_t DefaultMaxTransferSize = 64 * 1024;
    static constexpr uint32_t ChunkDelayMs = 5;

private:
    static bt_conn* ble_active_conn_;
    static size_t max_transfer_size_;
    static const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, gatt_service_);

    static Callbacks callbacks_;
    static k_mutex mutex_;
    static std::shared_ptr<BleSettingsStatus> status_;
    static std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer_;
    static BleSettingsCommandManager command_manager_;
    static atomic_t disconnected_during_read_;

    BleSettingsService() = default;
    ~BleSettingsService() = default;

    BleSettingsService(const BleSettingsService&) = delete;
    BleSettingsService& operator=(const BleSettingsService&) = delete;

    static void SetState(BleSettingsState new_state);
    static void HandleDataChunk(std::span<const uint8_t> data);
    static bool SendData(BleSettingsType type, std::span<const uint8_t> data);

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
        Callbacks callbacks,
        allocator_type allocator = std::pmr::get_default_resource(),
        size_t max_transfer_size = DefaultMaxTransferSize);

    static void BleConnected(bt_conn* conn);
    static void BleDisconnected(bt_conn* conn);

    [[nodiscard]] static BleSettingsStatus GetStatus();
    [[nodiscard]] static size_t GetMaxTransferSize() { return max_transfer_size_; }
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings

#include <algorithm>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble_settings_service.h"

LOG_MODULE_REGISTER(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings {

// Custom 128-bit UUIDs for the Configuration Service
// Base UUID: e7a1b2c3-d4e5-6f78-9a0b-c1d2e3f40000
#define BT_UUID_CONFIG_SERVICE_VAL \
    BT_UUID_128_ENCODE(0xe7a1b2c3, 0xd4e5, 0x6f78, 0x9a0b, 0xc1d2e3f40000)

#define BT_UUID_CONFIG_CONTROL_VAL \
    BT_UUID_128_ENCODE(0xe7a1b2c3, 0xd4e5, 0x6f78, 0x9a0b, 0xc1d2e3f40001)

#define BT_UUID_CONFIG_DATA_VAL \
    BT_UUID_128_ENCODE(0xe7a1b2c3, 0xd4e5, 0x6f78, 0x9a0b, 0xc1d2e3f40002)

#define BT_UUID_CONFIG_STATUS_VAL \
    BT_UUID_128_ENCODE(0xe7a1b2c3, 0xd4e5, 0x6f78, 0x9a0b, 0xc1d2e3f40003)

#define BT_UUID_CONFIG_SERVICE  BT_UUID_DECLARE_128(BT_UUID_CONFIG_SERVICE_VAL)
#define BT_UUID_CONFIG_CONTROL  BT_UUID_DECLARE_128(BT_UUID_CONFIG_CONTROL_VAL)
#define BT_UUID_CONFIG_DATA     BT_UUID_DECLARE_128(BT_UUID_CONFIG_DATA_VAL)
#define BT_UUID_CONFIG_STATUS   BT_UUID_DECLARE_128(BT_UUID_CONFIG_STATUS_VAL)

BleSettingsService::Callbacks BleSettingsService::callbacks_;
size_t BleSettingsService::max_transfer_size_{0};
k_mutex BleSettingsService::mutex_;
std::shared_ptr<BleSettingsStatus> BleSettingsService::status_{std::make_shared<BleSettingsStatus>()};
std::shared_ptr<std::pmr::vector<uint8_t>> BleSettingsService::transfer_buffer_;
BleSettingsCommandManager BleSettingsService::command_manager_{status_};
atomic_t BleSettingsService::disconnected_during_read_{0};

void BleSettingsService::Initialize(
    Callbacks callbacks,
    allocator_type allocator,
    size_t max_transfer_size) {

    k_mutex_init(&mutex_);

    k_mutex_lock(&mutex_, K_FOREVER);

    max_transfer_size_ = max_transfer_size;
    transfer_buffer_ = std::make_shared<std::pmr::vector<uint8_t>>(max_transfer_size_, uint8_t{0}, allocator);
    transfer_buffer_->resize(max_transfer_size_);

    callbacks_ = callbacks;
    if(callbacks_.on_state_change)
        status_->SetStateChangeHandler(callbacks_.on_state_change);

    command_manager_.Initialize(
        transfer_buffer_, {
            .on_config_write = callbacks_.on_config_write,
            .on_config_read = callbacks_.on_config_read
        });

    status_->Reset();

    k_mutex_unlock(&mutex_);
}

void BleSettingsService::BleConnected(bt_conn* conn) {
    k_mutex_lock(&mutex_, K_FOREVER);
    status_->Reset();
    k_mutex_unlock(&mutex_);
}

void BleSettingsService::BleDisconnected(bt_conn* conn) {
    // Signal any in-progress SendConfig loop to abort.
    // atomic_set(&disconnected_during_read_, 1);

    k_mutex_lock(&mutex_, K_FOREVER);
    status_->Reset();
    k_mutex_unlock(&mutex_);
}

BleSettingsStatus BleSettingsService::GetStatus() {
    k_mutex_lock(&mutex_, K_FOREVER);
    auto snapshot = *status_;
    k_mutex_unlock(&mutex_);

    return snapshot;
}

void BleSettingsService::HandleDataChunk(std::span<const uint8_t> data) {
    if(status_->GetState() != BleSettingsState::Writing) {
        LOG_ERR("Data chunk received while not writing");
        return;
    }

    // Guard against overflow using the buffer's own size, not just the
    // client-supplied total_bytes, so the check remains valid if
    // max_transfer_size_ and the validation in StartWrite ever diverge.
    if(status_->GetTransferredBytes() + data.size() > status_->GetTotalBytes()
        || status_->GetTransferredBytes() + data.size() > transfer_buffer_->size()) {

        LOG_ERR("Data overflow: would exceed %u bytes", status_->GetTotalBytes());
        status_->SetErrorCode(BleSettingsErrorCode::DataOverflow);
        status_->SetState(BleSettingsState::Error);

        return;
    }

    std::copy(data.begin(), data.end(), transfer_buffer_->begin() + status_->GetTransferredBytes());
    status_->SetTransferredBytes(status_->GetTransferredBytes() + data.size());

    LOG_DBG("Received chunk: %u bytes (total: %u/%u)",
        data.size(), status_->GetTransferredBytes(), status_->GetTotalBytes());
}

// TODO: Reimplement as a BLE callback
bool BleSettingsService::SendConfig(bt_conn* conn, BleSettingsType type) {
    // if(status_->GetState() != BleSettingsState::Idle) {
    //     LOG_ERR("Already in transfer");
    //     return false;
    // }

    // if(!callbacks_.on_config_read) {
    //     LOG_ERR("No read handler registered");
    //     return false;
    // }

    // size_t data_size = callbacks_.on_config_read(
    //     type, std::span(transfer_buffer_->data(), transfer_buffer_->size()));

    // if(data_size == 0) {
    //     LOG_ERR("Read handler returned no data");
    //     return false;
    // }

    // if(data_size > max_transfer_size_) {
    //     LOG_ERR("Config too large: %zu bytes", data_size);
    //     return false;
    // }

    // LOG_INF("Sending config: type=%u, size=%zu",
    //        static_cast<uint8_t>(type), data_size);

    // // RAII guard for the loop-local connection reference.
    // // This ensures unconditional unref on all exit paths.
    // struct ConnRefGuard {
    //     bt_conn* conn;
    //     ~ConnRefGuard() { if(conn) bt_conn_unref(conn); }
    // } loop_conn_guard{bt_conn_ref(conn)};
    // bt_conn* loop_conn = loop_conn_guard.conn;

    // status_->SetCurrentType(type);
    // status_->SetTotalBytes(data_size);
    // status_->SetTransferredBytes(0);
    // status_->SetErrorCode(BleSettingsErrorCode::None);
    // status_->SetState(BleSettingsState::Reading);

    // // Clear the disconnect flag before releasing the lock so we don't pick
    // // up a stale signal from a previous connection.
    // atomic_set(&disconnected_during_read_, 0);

    // k_mutex_unlock(&mutex_);

    // // --- From here, status_ is only updated under the lock at the end.
    // //     loop_conn is pinned by its own ref and remains valid regardless of
    // //     what Disconnected does to active_conn_. ---

    // // Lambda to handle cleanup and return result consistently.
    // auto finalize = [](bool success) {
    //     k_mutex_lock(&mutex_, K_FOREVER);

    //     // If Disconnected already reset the state machine while we were in the
    //     // loop, don't stomp on it again — just leave things idle.
    //     if(status_->GetState() == BleSettingsState::Reading) {
    //         if(!success) {
    //             status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
    //             status_->SetState(BleSettingsState::Error);
    //         }
    //         ResetTransfer();
    //     }

    //     k_mutex_unlock(&mutex_);
    //     return success;
    // };

    // const bt_gatt_attr* attr = bt_gatt_find_by_uuid(
    //     gatt_service_.attrs, gatt_service_.attr_count, BT_UUID_CONFIG_STATUS);

    // if(!attr) {
    //     LOG_ERR("Status characteristic not found");
    //     return finalize(false);
    // }

    // {
    //     struct {
    //         uint8_t cmd;
    //         uint8_t type;
    //         uint32_t size;
    //     } __attribute__((packed)) start_msg = {
    //         .cmd = static_cast<uint8_t>(BleSettingsCommandType::StartRead),
    //         .type = static_cast<uint8_t>(type),
    //         .size = static_cast<uint32_t>(data_size)
    //     };

    //     int err = bt_gatt_notify(loop_conn, attr, &start_msg, sizeof(start_msg));
    //     if(err) {
    //         LOG_ERR("Failed to send START_READ notification (err %d)", err);
    //         return finalize(false);
    //     }
    // }

    // {
    //     uint16_t mtu = bt_gatt_get_mtu(loop_conn);
    //     uint16_t chunk_size = std::min<uint16_t>(mtu - 3, 512);

    //     for(size_t offset = 0; offset < data_size; offset += chunk_size) {
    //         // Bail out early if Disconnected fired while we were looping.
    //         if(atomic_get(&disconnected_during_read_)) {
    //             LOG_INF("Aborting SendConfig: disconnected during transfer");
    //             return finalize(false);
    //         }

    //         size_t to_send = std::min<size_t>(chunk_size, data_size - offset);

    //         int err = bt_gatt_notify(loop_conn, attr,
    //             transfer_buffer_->data() + offset, to_send);

    //         if(err) {
    //             LOG_ERR("Notification failed at offset %zu (err %d)", offset, err);
    //             return finalize(false);
    //         }

    //         k_sleep(K_MSEC(ChunkDelayMs));
    //     }
    // }

    // {
    //     auto complete_cmd = static_cast<uint8_t>(BleSettingsCommandType::EndRead);
    //     int err = bt_gatt_notify(loop_conn, attr, &complete_cmd, 1);
    //     if(err) {
    //         LOG_ERR("Failed to send END_READ notification (err %d)", err);
    //         return finalize(false);
    //     }
    // }

    // LOG_INF("Config sent successfully");
    // return finalize(true);

    return true;
}

// GATT callbacks
// ==============

ssize_t ControlWriteCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    const void* buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    k_mutex_lock(&BleSettingsService::mutex_, K_FOREVER);
    BleSettingsService::command_manager_.Process(
        std::span(static_cast<const uint8_t*>(buf), len));
    k_mutex_unlock(&BleSettingsService::mutex_);

    return len;
}

ssize_t DataWriteCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    const void* buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    k_mutex_lock(&BleSettingsService::mutex_, K_FOREVER);
    BleSettingsService::HandleDataChunk(
        std::span(static_cast<const uint8_t*>(buf), len));
    k_mutex_unlock(&BleSettingsService::mutex_);

    return len;
}

ssize_t StatusReadCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    void* buf,
    uint16_t len,
    uint16_t offset) {

    auto status_data = BleSettingsService::GetStatus().GetStatusRaw();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, status_data.data(), status_data.size());
}

// GATT Service Definition
// =======================

// NOTE: This is supposed to be defined with BT_GATT_SERVICE_DEFINE(gatt_service_, ...) macro,
//       but in order to make gatt_service_ a class member macro has been expanded manually.
static const bt_gatt_attr gatt_attributes_[] = {
    BT_GATT_PRIMARY_SERVICE(BT_UUID_CONFIG_SERVICE),

    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_CONTROL,
                        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                        BT_GATT_PERM_WRITE,
                        nullptr, &ControlWriteCallback, nullptr),

    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_DATA,
                        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                        BT_GATT_PERM_WRITE,
                        nullptr, &DataWriteCallback, nullptr),

    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_STATUS,
                        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                        BT_GATT_PERM_READ_ENCRYPT,
                        &StatusReadCallback, nullptr, nullptr),
};

const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, BleSettingsService::gatt_service_) =
    BT_GATT_SERVICE(gatt_attributes_);

} // namespace eerie_leap::subsys::bluetooth::ble_settings

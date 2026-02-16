#include <algorithm>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble_configuration_service.h"

LOG_MODULE_REGISTER(ble_config_service);

namespace eerie_leap::subsys::bluetooth {

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

size_t BleConfigurationService::max_transfer_size_{0};
k_mutex BleConfigurationService::mutex_;
BleConfigurationServiceCallbacks BleConfigurationService::callbacks_;
BleConfigurationServiceStatus BleConfigurationService::status_{};
std::optional<std::pmr::vector<uint8_t>> BleConfigurationService::transfer_buffer_;
atomic_t BleConfigurationService::disconnected_during_read_{0};

void BleConfigurationService::Initialize(
    const BleConfigurationServiceCallbacks& callbacks,
    allocator_type allocator,
    size_t max_transfer_size) {

    k_mutex_lock(&mutex_, K_FOREVER);

    max_transfer_size_ = max_transfer_size;
    transfer_buffer_.emplace(max_transfer_size_, uint8_t{0}, allocator);
    transfer_buffer_.value().resize(max_transfer_size_);

    callbacks_ = callbacks;
    ResetTransfer();

    k_mutex_unlock(&mutex_);
}

void BleConfigurationService::BleConnected(bt_conn* conn) {
    k_mutex_lock(&mutex_, K_FOREVER);
    ResetTransfer();
    k_mutex_unlock(&mutex_);
}

void BleConfigurationService::BleDisconnected(bt_conn* conn) {
    // Signal any in-progress SendConfig loop to abort.
    atomic_set(&disconnected_during_read_, 1);

    k_mutex_lock(&mutex_, K_FOREVER);
    ResetTransfer();
    k_mutex_unlock(&mutex_);
}

BleConfigurationServiceStatus BleConfigurationService::GetStatus() {
    k_mutex_lock(&mutex_, K_FOREVER);
    auto snapshot = status_;
    k_mutex_unlock(&mutex_);

    return snapshot;
}

// Must be called with mutex_ held.
void BleConfigurationService::SetState(BleConfigServiceState new_state) {
    BleConfigServiceState old_state = status_.state;
    status_.state = new_state;

    if(callbacks_.on_state_change && old_state != new_state)
        callbacks_.on_state_change(old_state, new_state);
}

void BleConfigurationService::ResetTransfer() {
    status_ = {};
    SetState(BleConfigServiceState::Idle);
}

void BleConfigurationService::HandleControlCommand(bt_conn* conn, std::span<const uint8_t> data) {
    if(data.empty()) {
        LOG_ERR("Empty control command");
        return;
    }

    auto cmd = static_cast<BleConfigServiceCommand>(data[0]);

    k_mutex_lock(&mutex_, K_FOREVER);

    switch(cmd) {
        case BleConfigServiceCommand::StartWrite: {
            CommandStartWrite(conn, data);
            break;
        } case BleConfigServiceCommand::EndWrite: {
            CommandEndWrite(conn, data);
            break;
        } case BleConfigServiceCommand::Abort: {
            CommandAbort(conn, data);
            break;
        } case BleConfigServiceCommand::RequestRead: {
            CommandRequestRead(conn, data);
            break;
        } case BleConfigServiceCommand::StartRead: {
            CommandStartRead(conn, data);
            break;
        } case BleConfigServiceCommand::ReadComplete: {
            CommandReadComplete(conn, data);
            break;
        } default: {
            LOG_WRN("Unknown command: 0x%02x", data[0]);
            break;
        }
    }

    k_mutex_unlock(&mutex_);
}

// NOTE: Data format:
//       [0] - BleConfigServiceCommand::StartWrite
//       [1] - BleConfigServiceType
//       [2-5] - data size, uint32_t (little-endian)
void BleConfigurationService::CommandStartWrite(bt_conn* conn, std::span<const uint8_t> data) {
    if(data.size() < 6) {
        LOG_ERR("START_WRITE: insufficient data");
        status_.error_code = BleConfigServiceErrorCode::InsufficientData;
        SetState(BleConfigServiceState::Error);
        return;
    }

    auto type = static_cast<BleConfigServiceType>(data[1]);
    uint32_t size = data[2] | (data[3] << 8) |
                    (data[4] << 16) | (data[5] << 24);

    if(size > max_transfer_size_) {
        LOG_ERR("Config too large: %u bytes", size);
        status_.error_code = BleConfigServiceErrorCode::TransferTooLarge;
        SetState(BleConfigServiceState::Error);
        return;
    }

    LOG_INF("Starting write: type=%u, size=%u",
            static_cast<uint8_t>(type), size);

    status_.current_type = type;
    status_.total_bytes = size;
    status_.transferred_bytes = 0;
    status_.error_code = BleConfigServiceErrorCode::None;

    SetState(BleConfigServiceState::Writing);
}

// NOTE: Data format:
//       [0] - BleConfigServiceCommand::EndWrite
void BleConfigurationService::CommandEndWrite(bt_conn* conn, std::span<const uint8_t> data) {
    if(status_.state != BleConfigServiceState::Writing) {
        LOG_ERR("END_WRITE: not in writing state");
        status_.error_code = BleConfigServiceErrorCode::InvalidState;
        SetState(BleConfigServiceState::Error);
        return;
    }

    if(status_.transferred_bytes != status_.total_bytes) {
        LOG_ERR("Incomplete transfer: %u/%u bytes",
                status_.transferred_bytes, status_.total_bytes);
        status_.error_code = BleConfigServiceErrorCode::IncompleteTransfer;
        SetState(BleConfigServiceState::Error);
        return;
    }

    auto type = status_.current_type;
    uint32_t byte_count = status_.transferred_bytes;
    auto received_data = std::span(transfer_buffer_.value().data(), byte_count);

    if(callbacks_.on_config_write) {
        bool success = callbacks_.on_config_write(type, received_data);

        // A disconnect (or another command) may have fired while the
        // lock was released and already reset the state machine.  Only
        // act on the callback result if we are still in Writing.
        if(status_.state != BleConfigServiceState::Writing) {
            LOG_INF("END_WRITE: state changed during callback, ignoring result");
            return;
        }

        if(success) {
            LOG_INF("Config write successful");
            ResetTransfer();
        } else {
            LOG_ERR("Config write handler failed");
            status_.error_code = BleConfigServiceErrorCode::HandlerFailed;
            SetState(BleConfigServiceState::Error);
        }
    } else {
        LOG_WRN("No write handler registered");
        ResetTransfer();
    }
}

// NOTE: Data format:
//       [0] - BleConfigServiceCommand::Abort
void BleConfigurationService::CommandAbort(bt_conn* conn, std::span<const uint8_t> data) {
    LOG_INF("Transfer aborted");
    ResetTransfer();
}

void BleConfigurationService::CommandRequestRead(bt_conn* conn, std::span<const uint8_t> data) {
    if(data.size() < 2) {
        LOG_ERR("REQUEST_READ: insufficient data");
        return;
    }

    auto type = static_cast<BleConfigServiceType>(data[1]);
    LOG_INF("Read requested: type=%u", static_cast<uint8_t>(type));

    // TODO: Remove from here and implement BTE read callback
    // SendConfig(conn, type);
}

// NOTE: Data format:
//       [0] - BleConfigServiceCommand::StartRead
void BleConfigurationService::CommandStartRead(bt_conn* conn, std::span<const uint8_t> data) {

}

// NOTE: Data format:
//       [0] - BleConfigServiceCommand::ReadComplete
void BleConfigurationService::CommandReadComplete(bt_conn* conn, std::span<const uint8_t> data) {

}

void BleConfigurationService::HandleDataChunk(std::span<const uint8_t> data) {
    k_mutex_lock(&mutex_, K_FOREVER);

    if(status_.state != BleConfigServiceState::Writing) {
        LOG_ERR("Data chunk received while not writing");
        k_mutex_unlock(&mutex_);
        return;
    }

    // Guard against overflow using the buffer's own size, not just the
    // client-supplied total_bytes, so the check remains valid if
    // max_transfer_size_ and the validation in StartWrite ever diverge.
    if(status_.transferred_bytes + data.size() > status_.total_bytes
        || status_.transferred_bytes + data.size() > transfer_buffer_.value().size()) {

        LOG_ERR("Data overflow: would exceed %u bytes", status_.total_bytes);
        status_.error_code = BleConfigServiceErrorCode::DataOverflow;
        SetState(BleConfigServiceState::Error);

        k_mutex_unlock(&mutex_);
        return;
    }

    std::copy(data.begin(), data.end(), transfer_buffer_.value().begin() + status_.transferred_bytes);
    status_.transferred_bytes += data.size();

    LOG_DBG("Received chunk: %u bytes (total: %u/%u)",
        data.size(), status_.transferred_bytes, status_.total_bytes);

    k_mutex_unlock(&mutex_);
}

// TODO: Reimplement as a BLE callback
bool BleConfigurationService::SendConfig(bt_conn* conn, BleConfigServiceType type) {
    if(status_.state != BleConfigServiceState::Idle) {
        LOG_ERR("Already in transfer");
        return false;
    }

    if(!callbacks_.on_config_read) {
        LOG_ERR("No read handler registered");
        return false;
    }

    size_t data_size = callbacks_.on_config_read(
        type, std::span(transfer_buffer_.value()));

    if(data_size == 0) {
        LOG_ERR("Read handler returned no data");
        return false;
    }

    if(data_size > max_transfer_size_) {
        LOG_ERR("Config too large: %zu bytes", data_size);
        return false;
    }

    LOG_INF("Sending config: type=%u, size=%zu",
           static_cast<uint8_t>(type), data_size);

    // RAII guard for the loop-local connection reference.
    // This ensures unconditional unref on all exit paths.
    struct ConnRefGuard {
        bt_conn* conn;
        ~ConnRefGuard() { if(conn) bt_conn_unref(conn); }
    } loop_conn_guard{bt_conn_ref(conn)};
    bt_conn* loop_conn = loop_conn_guard.conn;

    status_.current_type = type;
    status_.total_bytes = data_size;
    status_.transferred_bytes = 0;
    status_.error_code = BleConfigServiceErrorCode::None;
    SetState(BleConfigServiceState::Reading);

    // Clear the disconnect flag before releasing the lock so we don't pick
    // up a stale signal from a previous connection.
    atomic_set(&disconnected_during_read_, 0);

    k_mutex_unlock(&mutex_);

    // --- From here, status_ is only updated under the lock at the end.
    //     loop_conn is pinned by its own ref and remains valid regardless of
    //     what Disconnected does to active_conn_. ---

    // Lambda to handle cleanup and return result consistently.
    auto finalize = [](bool success) {
        k_mutex_lock(&mutex_, K_FOREVER);

        // If Disconnected already reset the state machine while we were in the
        // loop, don't stomp on it again — just leave things idle.
        if(status_.state == BleConfigServiceState::Reading) {
            if(!success) {
                status_.error_code = BleConfigServiceErrorCode::NotificationFailed;
                SetState(BleConfigServiceState::Error);
            }
            ResetTransfer();
        }

        k_mutex_unlock(&mutex_);
        return success;
    };

    const bt_gatt_attr* attr = bt_gatt_find_by_uuid(
        gatt_service_.attrs, gatt_service_.attr_count, BT_UUID_CONFIG_STATUS);

    if(!attr) {
        LOG_ERR("Status characteristic not found");
        return finalize(false);
    }

    {
        struct {
            uint8_t cmd;
            uint8_t type;
            uint32_t size;
        } __attribute__((packed)) start_msg = {
            .cmd = static_cast<uint8_t>(BleConfigServiceCommand::StartRead),
            .type = static_cast<uint8_t>(type),
            .size = static_cast<uint32_t>(data_size)
        };

        int err = bt_gatt_notify(loop_conn, attr, &start_msg, sizeof(start_msg));
        if(err) {
            LOG_ERR("Failed to send START_READ notification (err %d)", err);
            return finalize(false);
        }
    }

    {
        uint16_t mtu = bt_gatt_get_mtu(loop_conn);
        uint16_t chunk_size = std::min<uint16_t>(mtu - 3, 512);

        for(size_t offset = 0; offset < data_size; offset += chunk_size) {
            // Bail out early if Disconnected fired while we were looping.
            if(atomic_get(&disconnected_during_read_)) {
                LOG_INF("Aborting SendConfig: disconnected during transfer");
                return finalize(false);
            }

            size_t to_send = std::min<size_t>(chunk_size, data_size - offset);

            int err = bt_gatt_notify(loop_conn, attr,
                transfer_buffer_.value().data() + offset, to_send);

            if(err) {
                LOG_ERR("Notification failed at offset %zu (err %d)", offset, err);
                return finalize(false);
            }

            k_sleep(K_MSEC(ChunkDelayMs));
        }
    }

    {
        auto complete_cmd = static_cast<uint8_t>(BleConfigServiceCommand::ReadComplete);
        int err = bt_gatt_notify(loop_conn, attr, &complete_cmd, 1);
        if(err) {
            LOG_ERR("Failed to send READ_COMPLETE notification (err %d)", err);
            return finalize(false);
        }
    }

    LOG_INF("Config sent successfully");
    return finalize(true);
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

    BleConfigurationService::HandleControlCommand(
        conn, std::span(static_cast<const uint8_t*>(buf), len));

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

    BleConfigurationService::HandleDataChunk(
        std::span(static_cast<const uint8_t*>(buf), len));

    return len;
}

ssize_t StatusReadCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    void* buf,
    uint16_t len,
    uint16_t offset) {

    auto status = BleConfigurationService::GetStatus();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &status, sizeof(status));
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
                        BT_GATT_PERM_READ,
                        &StatusReadCallback, nullptr, nullptr),
};

const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, BleConfigurationService::gatt_service_) =
    BT_GATT_SERVICE(gatt_attributes_);

} // namespace eerie_leap::subsys::bluetooth

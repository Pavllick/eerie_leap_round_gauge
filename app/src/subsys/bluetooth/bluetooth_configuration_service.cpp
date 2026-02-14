#include <algorithm>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "bluetooth_configuration_service.h"

LOG_MODULE_REGISTER(bt_config);

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

// Advertising data
// =================

const bt_data BluetoothConfigurationService::ad_[] = {
    // Flags: general discoverable, no BR/EDR
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

    // Full device name
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

const bt_le_adv_param* BluetoothConfigurationService::advertising_params_ = BT_LE_ADV_PARAM(
    BT_LE_ADV_OPT_CONN,
    BT_GAP_ADV_FAST_INT_MIN_2,
    BT_GAP_ADV_FAST_INT_MAX_2,
    nullptr
);

BluetoothConfigurationService& BluetoothConfigurationService::GetInstance() {
    static BluetoothConfigurationService instance;
    return instance;
}

bool BluetoothConfigurationService::Initialize(
    const Callbacks& callbacks,
    allocator_type allocator,
    size_t max_transfer_size) {

    k_mutex_lock(&mutex_, K_FOREVER);

    max_transfer_size_ = max_transfer_size;
    transfer_buffer_.emplace(max_transfer_size_, uint8_t{0}, allocator);
    transfer_buffer_.value().resize(max_transfer_size_);

    callbacks_ = callbacks;
    ResetTransfer();

    k_mutex_unlock(&mutex_);

    return InitializeBluetooth() == 0;
}

int BluetoothConfigurationService::InitializeBluetooth() {
    k_work_init_delayable(&adv_restart_work_, RestartAdvertisingWorkHandler);

    int err = bt_enable(nullptr);
    if(err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }

    LOG_INF("Bluetooth initialized");

    return StartAdvertising(K_NO_WAIT);
}

int BluetoothConfigurationService::StartAdvertising(k_timeout_t delay) {
    return k_work_schedule(&adv_restart_work_, delay);
}

void BluetoothConfigurationService::RestartAdvertisingWorkHandler(struct k_work* work) {
    bt_le_adv_stop();

    int err = bt_le_adv_start(advertising_params_, ad_, ARRAY_SIZE(ad_), nullptr, 0);
    if(err) {
        LOG_ERR("Failed to start advertising (err %d)", err);
        return;
    }

    LOG_INF("BLE advertising started");
}

void BluetoothConfigurationService::UpdateDataLength(bt_conn* conn) {
    struct bt_conn_le_data_len_param my_data_len = {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };

    int err = bt_conn_le_data_len_update(conn, &my_data_len);
    if(err) {
        LOG_ERR("data_len_update failed (err %d)", err);
    }
}

void BluetoothConfigurationService::UpdateMtu(bt_conn* conn) {
    bt_gatt_exchange_params exchange_params = {
        .func = GattExchangeParamsFunc,
    };

    int err = bt_gatt_exchange_mtu(conn, &exchange_params);
    if(err)
        LOG_ERR("bt_gatt_exchange_mtu failed (err %d)", err);
}

void BluetoothConfigurationService::GattExchangeParamsFunc(bt_conn* conn, uint8_t att_err, bt_gatt_exchange_params* params) {
    LOG_INF("MTU exchange %s", att_err == 0 ? "successful" : "failed");

    if(!att_err) {
        uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;   // 3 bytes used for Attribute headers.
        LOG_INF("New MTU: %d bytes", payload_mtu);
    }
}

Status BluetoothConfigurationService::GetStatus() {
    k_mutex_lock(&mutex_, K_FOREVER);
    Status snapshot = status_;
    k_mutex_unlock(&mutex_);

    return snapshot;
}

// Must be called with mutex_ held.
void BluetoothConfigurationService::SetState(State new_state) {
    State old_state = status_.state;
    status_.state = new_state;

    if(callbacks_.on_state_change && old_state != new_state)
        callbacks_.on_state_change(old_state, new_state);
}

void BluetoothConfigurationService::ResetTransfer() {
    status_ = {};
    SetState(State::Idle);
}

void BluetoothConfigurationService::HandleControlCommand(bt_conn* conn, std::span<const uint8_t> data) {
    if(data.empty()) {
        LOG_ERR("Empty control command");
        return;
    }

    auto cmd = static_cast<Command>(data[0]);

    k_mutex_lock(&mutex_, K_FOREVER);

    switch(cmd) {
        case Command::StartWrite: {
            if(data.size() < 6) {
                LOG_ERR("START_WRITE: insufficient data");
                status_.error_code = ErrorCode::InsufficientData;
                SetState(State::Error);
                k_mutex_unlock(&mutex_);
                return;
            }

            auto type = static_cast<ConfigType>(data[1]);
            uint32_t size = data[2] | (data[3] << 8) |
                          (data[4] << 16) | (data[5] << 24);

            if(size > max_transfer_size_) {
                LOG_ERR("Config too large: %u bytes", size);
                status_.error_code = ErrorCode::TransferTooLarge;
                SetState(State::Error);
                k_mutex_unlock(&mutex_);
                return;
            }

            LOG_INF("Starting write: type=%u, size=%u",
                   static_cast<uint8_t>(type), size);

            status_.current_type = type;
            status_.total_bytes = size;
            status_.transferred_bytes = 0;
            status_.error_code = ErrorCode::None;
            SetState(State::Writing);

            break;
        } case Command::EndWrite: {
            if(status_.state != State::Writing) {
                LOG_ERR("END_WRITE: not in writing state");
                status_.error_code = ErrorCode::InvalidState;
                SetState(State::Error);
                k_mutex_unlock(&mutex_);
                return;
            }

            if(status_.transferred_bytes != status_.total_bytes) {
                LOG_ERR("Incomplete transfer: %u/%u bytes",
                       status_.transferred_bytes, status_.total_bytes);
                status_.error_code = ErrorCode::IncompleteTransfer;
                SetState(State::Error);
                k_mutex_unlock(&mutex_);
                return;
            }

            ConfigType type = status_.current_type;
            uint32_t byte_count = status_.transferred_bytes;
            auto received_data = std::span(transfer_buffer_.value().data(), byte_count);

            if(callbacks_.on_config_write) {
                k_mutex_unlock(&mutex_);

                bool success = callbacks_.on_config_write(type, received_data);

                k_mutex_lock(&mutex_, K_FOREVER);

                // A disconnect (or another command) may have fired while the
                // lock was released and already reset the state machine.  Only
                // act on the callback result if we are still in Writing.
                if(status_.state != State::Writing) {
                    LOG_INF("END_WRITE: state changed during callback, ignoring result");
                    break;
                }

                if(success) {
                    LOG_INF("Config write successful");
                    ResetTransfer();
                } else {
                    LOG_ERR("Config write handler failed");
                    status_.error_code = ErrorCode::HandlerFailed;
                    SetState(State::Error);
                }
            } else {
                LOG_WRN("No write handler registered");
                ResetTransfer();
            }

            break;
        } case Command::Abort: {
            LOG_INF("Transfer aborted");
            ResetTransfer();

            break;
        } case Command::RequestRead: {
            if(data.size() < 2) {
                LOG_ERR("REQUEST_READ: insufficient data");
                k_mutex_unlock(&mutex_);
                return;
            }

            auto type = static_cast<ConfigType>(data[1]);
            LOG_INF("Read requested: type=%u", static_cast<uint8_t>(type));

            k_mutex_unlock(&mutex_);
            SendConfig(conn, type);

            return;
        } default: {
            LOG_WRN("Unknown command: 0x%02x", data[0]);
            break;
        }
    }

    k_mutex_unlock(&mutex_);
}

void BluetoothConfigurationService::HandleDataChunk(std::span<const uint8_t> data) {
    k_mutex_lock(&mutex_, K_FOREVER);

    if(status_.state != State::Writing) {
        LOG_ERR("Data chunk received while not writing");
        k_mutex_unlock(&mutex_);
        return;
    }

    // Guard against overflow using the buffer's own size, not just the
    // client-supplied total_bytes, so the check remains valid if
    // max_transfer_size_ and the validation in StartWrite ever diverge.
    if(status_.transferred_bytes + data.size() > status_.total_bytes ||
       status_.transferred_bytes + data.size() > transfer_buffer_.value().size()) {
        LOG_ERR("Data overflow: would exceed %u bytes", status_.total_bytes);
        status_.error_code = ErrorCode::DataOverflow;
        SetState(State::Error);
        k_mutex_unlock(&mutex_);
        return;
    }

    std::copy(data.begin(), data.end(),
             transfer_buffer_.value().begin() + status_.transferred_bytes);

    status_.transferred_bytes += data.size();

    LOG_DBG("Received chunk: %u bytes (total: %u/%u)",
           data.size(), status_.transferred_bytes, status_.total_bytes);

    k_mutex_unlock(&mutex_);
}

bool BluetoothConfigurationService::SendConfig(bt_conn* conn, ConfigType type) {
    k_mutex_lock(&mutex_, K_FOREVER);

    // Check notifications while already holding the lock, closing the TOCTOU
    // window that existed when this was checked before the lock was taken.
    if(!atomic_get(&notifications_enabled_)) {
        LOG_ERR("Notifications not enabled");
        k_mutex_unlock(&mutex_);
        return false;
    }

    if(status_.state != State::Idle) {
        LOG_ERR("Already in transfer");
        k_mutex_unlock(&mutex_);
        return false;
    }

    if(!callbacks_.on_config_read) {
        LOG_ERR("No read handler registered");
        k_mutex_unlock(&mutex_);
        return false;
    }

    size_t data_size = callbacks_.on_config_read(
        type, std::span(transfer_buffer_.value()));

    if(data_size == 0) {
        LOG_ERR("Read handler returned no data");
        k_mutex_unlock(&mutex_);
        return false;
    }

    if(data_size > max_transfer_size_) {
        LOG_ERR("Config too large: %zu bytes", data_size);
        k_mutex_unlock(&mutex_);
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
    status_.error_code = ErrorCode::None;
    SetState(State::Reading);

    // Clear the disconnect flag before releasing the lock so we don't pick
    // up a stale signal from a previous connection.
    atomic_set(&disconnected_during_read_, 0);

    k_mutex_unlock(&mutex_);

    // --- From here, status_ is only updated under the lock at the end.
    //     loop_conn is pinned by its own ref and remains valid regardless of
    //     what Disconnected does to active_conn_. ---

    // Lambda to handle cleanup and return result consistently.
    auto finalize = [this](bool success) {
        k_mutex_lock(&mutex_, K_FOREVER);

        // If Disconnected already reset the state machine while we were in the
        // loop, don't stomp on it again — just leave things idle.
        if(status_.state == State::Reading) {
            if(!success) {
                status_.error_code = ErrorCode::NotificationFailed;
                SetState(State::Error);
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
            .cmd = static_cast<uint8_t>(Command::StartRead),
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
        auto complete_cmd = static_cast<uint8_t>(Command::ReadComplete);
        int err = bt_gatt_notify(loop_conn, attr, &complete_cmd, 1);
        if(err) {
            LOG_ERR("Failed to send READ_COMPLETE notification (err %d)", err);
            return finalize(false);
        }
    }

    LOG_INF("Config sent successfully");
    return finalize(true);
}

// Connection callbacks
// ====================

void Connected(bt_conn* conn, uint8_t err) {
    if(err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    LOG_INF("Connected: %s", addr);

    auto& svc = BluetoothConfigurationService::GetInstance();
    k_mutex_lock(&svc.mutex_, K_FOREVER);
    if(svc.active_conn_)
        bt_conn_unref(svc.active_conn_);
    svc.active_conn_ = bt_conn_ref(conn);

    svc.ResetTransfer();

    k_msleep(1000);
    svc.UpdateDataLength(conn);
    svc.UpdateMtu(conn);
    k_mutex_unlock(&svc.mutex_);
}

void Disconnected(bt_conn* conn, uint8_t reason) {
    LOG_INF("Disconnected (reason %u)", reason);

    auto& svc = BluetoothConfigurationService::GetInstance();

    // Signal any in-progress SendConfig loop to abort.
    atomic_set(&svc.disconnected_during_read_, 1);

    k_mutex_lock(&svc.mutex_, K_FOREVER);
    svc.ResetTransfer();
    if(svc.active_conn_)
        bt_conn_unref(svc.active_conn_);
    svc.active_conn_ = nullptr;
    k_mutex_unlock(&svc.mutex_);

    // Defer advertising restart to allow BT stack to fully clean up
    svc.StartAdvertising(K_MSEC(100));
}

void ParamertersUpdated(bt_conn* conn, uint16_t interval, uint16_t latency, uint16_t timeout) {
    double connection_interval_ms = interval * 1.25;
    uint16_t supervision_timeout_ms = timeout * 10;

    LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms",
        connection_interval_ms, latency, supervision_timeout_ms);
}

// NOTE: Any logging from this callback will cause a crash.
//       Thus it's disabled.
void DataLengthUpdated(bt_conn* conn, bt_conn_le_data_len_info* info) {
    uint16_t tx_len = info->tx_max_len;
    uint16_t tx_time = info->tx_max_time;
    uint16_t rx_len = info->rx_max_len;
    uint16_t rx_time = info->rx_max_time;

    LOG_INF("Data length updated. Length %d/%d bytes, time %d/%d us",
        tx_len, rx_len, tx_time, rx_time);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = &Connected,
    .disconnected = &Disconnected,
    .le_param_updated = &ParamertersUpdated,
    // .le_data_len_updated = &DataLengthUpdated // See note at the method implementation
};

// GATT callbacks
// ==============

ssize_t ControlWriteCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    const void* buf, uint16_t len,
    uint16_t offset, uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    BluetoothConfigurationService::GetInstance().HandleControlCommand(
        conn, std::span(static_cast<const uint8_t*>(buf), len));

    return len;
}

ssize_t DataWriteCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    const void* buf, uint16_t len,
    uint16_t offset, uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    BluetoothConfigurationService::GetInstance().HandleDataChunk(
        std::span(static_cast<const uint8_t*>(buf), len));

    return len;
}

ssize_t StatusReadCallback(
    bt_conn* conn,
    const bt_gatt_attr* attr,
    void* buf, uint16_t len,
    uint16_t offset) {

    auto status = BluetoothConfigurationService::GetInstance().GetStatus();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &status, sizeof(status));
}

void NotifyCccChanged(const bt_gatt_attr* attr, uint16_t value) {
    atomic_set(&BluetoothConfigurationService::GetInstance().notifications_enabled_,
        (value == BT_GATT_CCC_NOTIFY) ? 1 : 0);

    LOG_INF("Notifications %s",
        atomic_get(&BluetoothConfigurationService::GetInstance().notifications_enabled_)
            ? "enabled"
            : "disabled");
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
    BT_GATT_CCC(&NotifyCccChanged, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
};

const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, BluetoothConfigurationService::gatt_service_) =
    BT_GATT_SERVICE(gatt_attributes_);

} // namespace eerie_leap::subsys::bluetooth

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

// Work handler for deferred advertising restart
void AdvRestartWorkHandler(struct k_work* work) {
    auto& svc = BluetoothConfigurationService::GetInstance();
    int err = svc.StartExtendedAdvertising();
    if(err)
        LOG_ERR("Failed to restart advertising (err %d)", err);
}

// GATT callbacks

ssize_t ControlWriteCallback(bt_conn* conn,
    const bt_gatt_attr* attr,
    const void* buf, uint16_t len,
    uint16_t offset, uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    BluetoothConfigurationService::GetInstance().HandleControlCommand(
        conn, std::span(static_cast<const uint8_t*>(buf), len));

    return len;
}

ssize_t DataWriteCallback(bt_conn* conn,
    const bt_gatt_attr* attr,
    const void* buf, uint16_t len,
    uint16_t offset, uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    BluetoothConfigurationService::GetInstance().HandleDataChunk(
        std::span(static_cast<const uint8_t*>(buf), len));

    return len;
}

ssize_t StatusReadCallback(bt_conn* conn,
    const bt_gatt_attr* attr,
    void* buf, uint16_t len,
    uint16_t offset) {

    auto status = BluetoothConfigurationService::GetInstance().GetStatus();
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                            &status, sizeof(status));
}

void NotifyCccChanged(const bt_gatt_attr* attr, uint16_t value) {
    atomic_set(&BluetoothConfigurationService::GetInstance().notifications_enabled_,
        (value == BT_GATT_CCC_NOTIFY) ? 1 : 0);

    LOG_INF("Notifications %s",
        atomic_get(&BluetoothConfigurationService::GetInstance().notifications_enabled_)
            ? "enabled" : "disabled");
}

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
    svc.ResetTransferLocked();
    k_mutex_unlock(&svc.mutex_);
}

void Disconnected(bt_conn* conn, uint8_t reason) {
    LOG_INF("Disconnected (reason %u)", reason);

    auto& svc = BluetoothConfigurationService::GetInstance();

    // Signal any in-progress SendConfig loop to abort.
    atomic_set(&svc.disconnected_during_read_, 1);

    k_mutex_lock(&svc.mutex_, K_FOREVER);
    svc.ResetTransferLocked();
    k_mutex_unlock(&svc.mutex_);

    // Defer advertising restart to allow BT stack to fully clean up
    svc.StartAdvertising(K_MSEC(100));
}

// GATT Service Definition

BT_GATT_SERVICE_DEFINE(config_svc,
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
);

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = &Connected,
    .disconnected = &Disconnected,
};

// Advertising data

static const bt_data ad[] = {
    // Flags: general discoverable, no BR/EDR
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

    // Full device name
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static const bt_le_adv_param extended_advertising_params = BT_LE_ADV_PARAM_INIT(
    BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_EXT_ADV,
    BT_GAP_ADV_FAST_INT_MIN_2,
    BT_GAP_ADV_FAST_INT_MAX_2,
    nullptr
);

int BluetoothConfigurationService::StartAdvertising(k_timeout_t delay) {
    return k_work_schedule(&adv_restart_work_, delay);
}

int BluetoothConfigurationService::StartExtendedAdvertising() {
    if(extended_advertising_ == nullptr) {
        int err = bt_le_ext_adv_create(&extended_advertising_params, nullptr, &extended_advertising_);
        if(err) {
            LOG_ERR("Failed to create extended advertising set (err %d)", err);
            return err;
        }

        err = bt_le_ext_adv_set_data(extended_advertising_, ad, ARRAY_SIZE(ad), nullptr, 0);
        if(err) {
            LOG_ERR("Failed to set extended advertising data (err %d)", err);
            return err;
        }
    } else {
        // Stop advertising before restarting (may already be stopped, ignore error)
        bt_le_ext_adv_stop(extended_advertising_);
    }

    int err = bt_le_ext_adv_start(extended_advertising_, BT_LE_EXT_ADV_START_DEFAULT);
    if(err) {
        LOG_ERR("Failed to start extended advertising (err %d)", err);
        return err;
    }

    LOG_INF("Extended advertising started");
    return 0;
}

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
    ResetTransferLocked();

    k_mutex_unlock(&mutex_);

    return InitializeBluetooth() == 0;
}

int BluetoothConfigurationService::InitializeBluetooth() {
    k_work_init_delayable(&adv_restart_work_, AdvRestartWorkHandler);

    int err = bt_enable(nullptr);
    if(err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }

    LOG_INF("Bluetooth initialized");

    return StartAdvertising(K_NO_WAIT);
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

// Must be called with mutex_ held. Unrefs active_conn_ if set.
void BluetoothConfigurationService::ResetTransferLocked() {
    if(active_conn_) {
        bt_conn_unref(active_conn_);
        active_conn_ = nullptr;
    }

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

            // Replace any existing conn reference.
            if(active_conn_)
                bt_conn_unref(active_conn_);
            active_conn_ = bt_conn_ref(conn);

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
                    ResetTransferLocked();
                } else {
                    LOG_ERR("Config write handler failed");
                    status_.error_code = ErrorCode::HandlerFailed;
                    SetState(State::Error);
                }
            } else {
                LOG_WRN("No write handler registered");
                ResetTransferLocked();
            }

            break;
        } case Command::Abort: {
            LOG_INF("Transfer aborted");
            ResetTransferLocked();

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

    // Store a ref so the conn stays valid for the duration of the transfer.
    if(active_conn_)
        bt_conn_unref(active_conn_);
    active_conn_ = bt_conn_ref(conn);

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
            ResetTransferLocked();
        }

        k_mutex_unlock(&mutex_);
        return success;
    };

    const bt_gatt_attr* attr = bt_gatt_find_by_uuid(
        config_svc.attrs, config_svc.attr_count, BT_UUID_CONFIG_STATUS);

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

} // namespace eerie_leap::subsys::bluetooth

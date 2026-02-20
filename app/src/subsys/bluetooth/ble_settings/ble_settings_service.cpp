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

bt_conn* BleSettingsService::ble_active_conn_{nullptr};
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
            .on_config_read = callbacks_.on_config_read,
            .on_send = SendData
        });

    status_->Reset();

    k_mutex_unlock(&mutex_);
}

void BleSettingsService::BleConnected(bt_conn* conn) {
    k_mutex_lock(&mutex_, K_FOREVER);
    if(ble_active_conn_)
        bt_conn_unref(ble_active_conn_);
    ble_active_conn_ = bt_conn_ref(conn);

    status_->Reset();
    k_mutex_unlock(&mutex_);
}

void BleSettingsService::BleDisconnected(bt_conn* conn) {
    // Signal any in-progress SendConfig loop to abort.
    // atomic_set(&disconnected_during_read_, 1);

    k_mutex_lock(&mutex_, K_FOREVER);
    if(ble_active_conn_)
        bt_conn_unref(ble_active_conn_);
    ble_active_conn_ = nullptr;

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

bool BleSettingsService::SendData(BleSettingsType type, std::span<const uint8_t> data) {
    if(status_->GetState() != BleSettingsState::Reading) {
        LOG_ERR("Send: not in Reading state");
        return false;
    }

    if(!ble_active_conn_) {
        LOG_ERR("Send: no active connection");
        status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
        status_->SetState(BleSettingsState::Error);

        return false;
    }

    if(data.empty()) {
        LOG_ERR("Send: empty data");
        status_->SetErrorCode(BleSettingsErrorCode::HandlerFailed);
        status_->SetState(BleSettingsState::Error);

        return false;
    }

    // Find characteristics
    const bt_gatt_attr* status_attr = bt_gatt_find_by_uuid(
        gatt_service_.attrs, gatt_service_.attr_count, BT_UUID_CONFIG_STATUS);
    const bt_gatt_attr* data_attr = bt_gatt_find_by_uuid(
        gatt_service_.attrs, gatt_service_.attr_count, BT_UUID_CONFIG_DATA);

    if(!status_attr || !data_attr) {
        LOG_ERR("Send: characteristics not found");
        status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
        status_->SetState(BleSettingsState::Error);

        return false;
    }

    // Update status
    status_->SetTotalBytes(data.size());
    status_->SetTransferredBytes(0);
    status_->SetErrorCode(BleSettingsErrorCode::None);

    LOG_INF("Sending config: type=%u, size=%zu", static_cast<uint8_t>(type), data.size());

    bool success = true;

    // 1. Send StartRead notification
    {
        struct {
            uint8_t cmd;
            uint8_t type;
            uint32_t size;
        } __attribute__((packed)) start_msg = {
            .cmd = static_cast<uint8_t>(BleSettingsCommandType::StartRead),
            .type = static_cast<uint8_t>(type),
            .size = static_cast<uint32_t>(data.size())
        };

        int err = bt_gatt_notify(ble_active_conn_, status_attr, &start_msg, sizeof(start_msg));
        if(err) {
            LOG_ERR("Failed to send StartRead notification (err %d)", err);
            success = false;
        }
    }

    // 2. Send data chunks
    if(success) {
        uint16_t mtu = bt_gatt_get_mtu(ble_active_conn_);
        uint16_t chunk_size = std::min<uint16_t>(mtu - 3, 512);

        for(size_t offset = 0; offset < data.size() && success; offset += chunk_size) {
            size_t to_send = std::min<size_t>(chunk_size, data.size() - offset);

            int err = bt_gatt_notify(ble_active_conn_, data_attr,
                data.data() + offset, to_send);

            if(err) {
                LOG_ERR("Notification failed at offset %zu (err %d)", offset, err);
                success = false;
                break;
            }

            if(status_->GetState() == BleSettingsState::Reading) {
                status_->SetTransferredBytes(offset + to_send);
            }

            // Small delay for flow control
            k_sleep(K_MSEC(10));
        }
    }

    // 3. Send EndRead notification
    if(success) {
        uint8_t end_cmd = static_cast<uint8_t>(BleSettingsCommandType::EndRead);
        int err = bt_gatt_notify(ble_active_conn_, status_attr, &end_cmd, 1);
        if(err) {
            LOG_ERR("Failed to send EndRead notification (err %d)", err);
            success = false;
        }
    }

    // Only update if still in Reading state (disconnect might have reset)
    if(status_->GetState() == BleSettingsState::Reading) {
        if(success) {
            LOG_INF("Config sent successfully");
            status_->Reset();
        } else {
            status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
            status_->SetState(BleSettingsState::Error);
        }
    }

    return success;
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
                        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP | BT_GATT_CHRC_NOTIFY,
                        BT_GATT_PERM_WRITE,
                        nullptr, &DataWriteCallback, nullptr),
    BT_GATT_CCC(nullptr, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_STATUS,
                        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                        BT_GATT_PERM_READ_ENCRYPT,
                        &StatusReadCallback, nullptr, nullptr),
    BT_GATT_CCC(nullptr, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
};

const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, BleSettingsService::gatt_service_) =
    BT_GATT_SERVICE(gatt_attributes_);

} // namespace eerie_leap::subsys::bluetooth::ble_settings

#include <algorithm>
#include <exception>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble_settings_command/ble_settings_command_start_read.h"
#include "ble_settings_command/ble_settings_command_end_read.h"
#include "ble_settings_service.h"

LOG_MODULE_REGISTER(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings {

using namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command;

#define BT_UUID_SETTINGS_CONTROL_VAL BT_UUID_SETTINGS_SERVICE_ENCODE(1)
#define BT_UUID_SETTINGS_DATA_VAL BT_UUID_SETTINGS_SERVICE_ENCODE(2)
#define BT_UUID_SETTINGS_STATUS_VAL BT_UUID_SETTINGS_SERVICE_ENCODE(3)

#define BT_UUID_SETTINGS_SERVICE  BT_UUID_DECLARE_128(BT_UUID_SETTINGS_SERVICE_VAL)
#define BT_UUID_SETTINGS_CONTROL  BT_UUID_DECLARE_128(BT_UUID_SETTINGS_CONTROL_VAL)
#define BT_UUID_SETTINGS_DATA     BT_UUID_DECLARE_128(BT_UUID_SETTINGS_DATA_VAL)
#define BT_UUID_SETTINGS_STATUS   BT_UUID_DECLARE_128(BT_UUID_SETTINGS_STATUS_VAL)

bt_conn* BleSettingsService::ble_active_conn_{nullptr};
BleSettingsService::Callbacks BleSettingsService::callbacks_;
size_t BleSettingsService::max_transfer_size_{0};
k_mutex BleSettingsService::mutex_;
std::shared_ptr<BleSettingsStatus> BleSettingsService::status_{std::make_shared<BleSettingsStatus>()};
std::shared_ptr<std::pmr::vector<uint8_t>> BleSettingsService::transfer_buffer_;
BleSettingsCommandManager BleSettingsService::command_manager_{status_};
atomic_t BleSettingsService::disconnected_during_read_{0};

void BleSettingsService::Initialize(
    const Callbacks& callbacks,
    allocator_type allocator,
    size_t max_transfer_size) {

    k_mutex_init(&mutex_);

    k_mutex_lock(&mutex_, K_FOREVER);

    max_transfer_size_ = max_transfer_size;
    transfer_buffer_ = std::make_shared<std::pmr::vector<uint8_t>>(max_transfer_size_, allocator);
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

void BleSettingsService::BleDisconnected([[maybe_unused]] bt_conn* conn) {
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

    std::ranges::copy(data, transfer_buffer_->begin() + status_->GetTransferredBytes());
    status_->SetTransferredBytes(status_->GetTransferredBytes() + data.size());

    LOG_DBG("Received chunk: %u bytes (total: %u/%u)",
        data.size(), status_->GetTransferredBytes(), status_->GetTotalBytes());
}

bool BleSettingsService::SendData(uint8_t settings_id, std::span<const uint8_t> data) {
    if(status_->GetState() != BleSettingsState::Reading) {
        LOG_ERR("SendData: not in Reading state");
        return false;
    }

    if(!ble_active_conn_) {
        LOG_ERR("SendData: no active connection");
        status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
        status_->SetState(BleSettingsState::Error);

        return false;
    }

    if(data.empty()) {
        LOG_ERR("SendData: empty data");
        status_->SetErrorCode(BleSettingsErrorCode::HandlerFailed);
        status_->SetState(BleSettingsState::Error);

        return false;
    }

    // Find characteristics
    const bt_gatt_attr* status_attr = bt_gatt_find_by_uuid(
        gatt_service_.attrs, (uint16_t)gatt_service_.attr_count, BT_UUID_SETTINGS_STATUS);
    const bt_gatt_attr* data_attr = bt_gatt_find_by_uuid(
        gatt_service_.attrs, (uint16_t)gatt_service_.attr_count, BT_UUID_SETTINGS_DATA);

    if(!status_attr || !data_attr) {
        LOG_ERR("SendData: BLE characteristics not found");
        status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
        status_->SetState(BleSettingsState::Error);

        return false;
    }

    // Update status
    status_->SetTotalBytes(data.size());
    status_->SetTransferredBytes(0);
    status_->SetErrorCode(BleSettingsErrorCode::None);

    LOG_INF("SendData: Sending config: id=%u, size=%zu", settings_id, data.size());

    bool success = true;

    // 1. Send StartRead notification
    {
        auto start_msg = BleSettingsCommandStartRead::Create(settings_id, data.size());
        int err = bt_gatt_notify(ble_active_conn_, status_attr, start_msg.data(), (uint16_t)start_msg.size());
        if(err) {
            LOG_ERR("SendData: Failed to send StartRead notification (err %d)", err);
            success = false;
        }
    }

    // 2. Send data chunks
    if(success) {
        uint16_t mtu = bt_gatt_get_mtu(ble_active_conn_);
        if(mtu <= 3) {
            LOG_ERR("SendData: invalid MTU %u", mtu);
            status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
            status_->SetState(BleSettingsState::Error);

            return false;
        }

        uint16_t chunk_size = std::min<uint16_t>(mtu - 3, 512);

        int retry_count = 0;
        size_t offset = 0;

        while(offset < data.size() && success) {
            size_t to_send = std::min<size_t>(chunk_size, data.size() - offset);

            int err = bt_gatt_notify(ble_active_conn_, data_attr, data.data() + offset, (uint16_t)to_send);
            if(err) {
                retry_count++;

                if(retry_count >= MAX_TRANSFER_RETRIES) {
                    success = false;
                    break;
                }

                LOG_ERR("SendData: Notification failed at offset %zu (err %d). Retry %d/%d",
                    offset, err, retry_count, MAX_TRANSFER_RETRIES);

                k_sleep(K_MSEC(100));

                continue;
            }

            if(status_->GetState() == BleSettingsState::Reading) {
                status_->SetTransferredBytes(offset + to_send);
            } else {
                LOG_ERR("SendData: BLE connection state changed to %u during transfer",
                    static_cast<unsigned int>(status_->GetState()));
                success = false;
                break;
            }

            offset += to_send;
            retry_count = 0;
        }
    }

    // 3. Send EndRead notification
    if(success) {
        auto end_msg = BleSettingsCommandEndRead::Create();
        int err = bt_gatt_notify(ble_active_conn_, status_attr, end_msg.data(), (uint16_t)end_msg.size());
        if(err) {
            LOG_ERR("SendData: Failed to send EndRead notification (err %d)", err);
            success = false;
        }
    }

    // Only update if still in Reading state (disconnect might have reset)
    if(status_->GetState() == BleSettingsState::Reading) {
        if(success) {
            LOG_INF("SendData: Config sent successfully");
            status_->Reset();
        } else {
            LOG_ERR("SendData: Failed to send config");
            status_->SetErrorCode(BleSettingsErrorCode::NotificationFailed);
            status_->SetState(BleSettingsState::Error);
        }
    }

    return success;
}

// GATT callbacks
// ==============

ssize_t ControlWriteCallback(
    [[maybe_unused]] bt_conn* conn,
    [[maybe_unused]] const bt_gatt_attr* attr,
    const void* buf,
    uint16_t len,
    uint16_t offset,
    [[maybe_unused]] uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    k_mutex_lock(&BleSettingsService::mutex_, K_FOREVER);
    try {
        BleSettingsService::command_manager_.Process(
            std::span(static_cast<const uint8_t*>(buf), len));
    } catch(const std::exception& e) {
        LOG_ERR("Control command processing threw: %s", e.what());
    } catch(...) {
        LOG_ERR("Control command processing threw an unknown exception");
    }
    k_mutex_unlock(&BleSettingsService::mutex_);

    return len;
}

ssize_t DataWriteCallback(
    [[maybe_unused]] bt_conn* conn,
    [[maybe_unused]] const bt_gatt_attr* attr,
    const void* buf,
    uint16_t len,
    uint16_t offset,
    [[maybe_unused]] uint8_t flags) {

    if(offset != 0)
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    k_mutex_lock(&BleSettingsService::mutex_, K_FOREVER);
    try {
        BleSettingsService::HandleDataChunk(
            std::span(static_cast<const uint8_t*>(buf), len));
    } catch(const std::exception& e) {
        LOG_ERR("Data chunk handling threw: %s", e.what());
    } catch(...) {
        LOG_ERR("Data chunk handling threw an unknown exception");
    }
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
    return bt_gatt_attr_read(conn, attr, buf, len, offset, status_data.data(), (uint16_t)status_data.size());
}

// GATT Service Definition
// =======================

// NOTE: This is supposed to be defined with BT_GATT_SERVICE_DEFINE(gatt_service_, ...) macro,
//       but in order to make gatt_service_ a class member macro has been expanded manually.
static const bt_gatt_attr gatt_attributes_[] = {
    BT_GATT_PRIMARY_SERVICE(BT_UUID_SETTINGS_SERVICE),

    BT_GATT_CHARACTERISTIC(BT_UUID_SETTINGS_CONTROL,
                        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                        BT_GATT_PERM_WRITE,
                        nullptr, &ControlWriteCallback, nullptr),

    BT_GATT_CHARACTERISTIC(BT_UUID_SETTINGS_DATA,
                        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP | BT_GATT_CHRC_NOTIFY,
                        BT_GATT_PERM_WRITE,
                        nullptr, &DataWriteCallback, nullptr),
    BT_GATT_CCC(nullptr, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    BT_GATT_CHARACTERISTIC(BT_UUID_SETTINGS_STATUS,
                        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                        BT_GATT_PERM_READ_ENCRYPT,
                        &StatusReadCallback, nullptr, nullptr),
    BT_GATT_CCC(nullptr, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
};

const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, BleSettingsService::gatt_service_) =
    BT_GATT_SERVICE(gatt_attributes_);

} // namespace eerie_leap::subsys::bluetooth::ble_settings

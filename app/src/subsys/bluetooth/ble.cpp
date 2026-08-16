#include <algorithm>
#include <exception>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include "ble.h"

LOG_MODULE_REGISTER(ble);

namespace eerie_leap::subsys::bluetooth {

// Advertising data
// =================

AdBuffer Ble::ad_;
AdBuffer Ble::sd_;

void Ble::UpdateAdvertisingData(AdBuffer&& ad) {
    ad_ = std::move(ad);
}

void Ble::UpdateScanResponseData(AdBuffer&& sd) {
    sd_ = std::move(sd);
}

const bt_le_adv_param* Ble::advertising_params_ = BT_LE_ADV_PARAM(
    BT_LE_ADV_OPT_CONN,
    BT_GAP_ADV_FAST_INT_MIN_2,
    BT_GAP_ADV_FAST_INT_MAX_2,
    nullptr
);

bt_conn* Ble::active_conn_{nullptr};
k_mutex Ble::conn_mutex_;
k_work_delayable Ble::adv_restart_work_;

k_work_delayable Ble::connected_cb_work_;
k_work_delayable Ble::security_update_work_;
k_work_delayable Ble::data_length_update_work_;
k_work_delayable Ble::pairing_started_work_;
k_work_delayable Ble::pairing_finished_work_;

std::unordered_map<int, Ble::ConnectedHandler> Ble::connected_handlers_;
std::unordered_map<int, Ble::DisconnectedHandler> Ble::disconnected_handlers_;
std::unordered_map<int, Ble::PairingStartedHandler> Ble::pairing_started_handlers_;
std::unordered_map<int, Ble::PairingFinishedHandler> Ble::pairing_finished_handlers_;

bool Ble::Initialize() {
    k_mutex_init(&conn_mutex_);

    k_work_init_delayable(&adv_restart_work_, RestartAdvertisingWorkHandler);
    k_work_init_delayable(&connected_cb_work_, ConnectedCbWorkHandler);
    k_work_init_delayable(&security_update_work_, SecurityUpdateWorkHandler);
    k_work_init_delayable(&data_length_update_work_, DataLengthUpdateWorkHandler);
    k_work_init_delayable(&pairing_started_work_, PairingStartedWorkHandler);
    k_work_init_delayable(&pairing_finished_work_, PairingFinishedWorkHandler);

    int err = bt_enable(nullptr);
    if(err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return false;
    }

    if(IS_ENABLED(CONFIG_BT_SETTINGS)) {
        err = settings_load();
        if(err)
            LOG_ERR("Failed to load Bluetooth settings (err %d)", err);
    }

    LOG_INF("Bluetooth initialized");

    return true;
}

bool Ble::Start() {
    return StartAdvertising() == 0;
}

// Advertising
// ===========

int Ble::StartAdvertising() {
    return k_work_reschedule(&adv_restart_work_, K_NO_WAIT);
}

void Ble::RestartAdvertisingWorkHandler(struct k_work* work) {
    bt_le_adv_stop();

    int err = bt_le_adv_start(
        advertising_params_,
        ad_.empty() ? nullptr : ad_.data(), ad_.size(),
        sd_.empty() ? nullptr : sd_.data(), sd_.size());
    if(err == -EALREADY) {
        LOG_DBG("BLE advertising already active");
        return;
    }
    if(err) {
        LOG_ERR("Failed to start advertising (err %d)", err);
        return;
    }

    LOG_INF("BLE advertising started");
}

// Connection access
// =================

bt_conn* Ble::AcquireActiveConn() {
    k_mutex_lock(&conn_mutex_, K_FOREVER);
    bt_conn* conn = active_conn_ != nullptr ? bt_conn_ref(active_conn_) : nullptr;
    k_mutex_unlock(&conn_mutex_);

    return conn;
}

// Connection configuration
// ========================

void Ble::UpdateDataLength(bt_conn* conn) {
    struct bt_conn_le_data_len_param my_data_len = {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };

    int err = bt_conn_le_data_len_update(conn, &my_data_len);
    if(err) {
        LOG_ERR("data_len_update failed (err %d)", err);
    }
}

void Ble::UpdateMtu(bt_conn* conn) {
    bt_gatt_exchange_params exchange_params = {
        .func = GattExchangeParamsFunc,
    };

    int err = bt_gatt_exchange_mtu(conn, &exchange_params);
    if(err)
        LOG_ERR("bt_gatt_exchange_mtu failed (err %d)", err);
}

void Ble::GattExchangeParamsFunc(bt_conn* conn, uint8_t att_err, bt_gatt_exchange_params* params) {
    LOG_INF("MTU exchange %s", att_err == 0 ? "successful" : "failed");

    if(!att_err) {
        uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;   // 3 bytes used for Attribute headers.
        LOG_INF("New MTU: %d bytes", payload_mtu);
    }
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

    k_mutex_lock(&Ble::conn_mutex_, K_FOREVER);
    if(Ble::active_conn_)
        bt_conn_unref(Ble::active_conn_);
    Ble::active_conn_ = bt_conn_ref(conn);
    k_mutex_unlock(&Ble::conn_mutex_);

    k_work_reschedule(&Ble::connected_cb_work_, K_NO_WAIT);
    k_work_reschedule(&Ble::pairing_started_work_, K_NO_WAIT);
    k_work_reschedule(&Ble::security_update_work_, K_NO_WAIT);
}

void Disconnected(bt_conn* conn, uint8_t reason) {
    LOG_INF("Disconnected (reason %u)", reason);

    for(auto& [_, handler] : Ble::disconnected_handlers_) {
        try {
            handler(conn);
        } catch(const std::exception& e) {
            LOG_ERR("Disconnected handler threw: %s", e.what());
        } catch(...) {
            LOG_ERR("Disconnected handler threw an unknown exception");
        }
    }

    k_mutex_lock(&Ble::conn_mutex_, K_FOREVER);
    if(Ble::active_conn_)
        bt_conn_unref(Ble::active_conn_);
    Ble::active_conn_ = nullptr;
    k_mutex_unlock(&Ble::conn_mutex_);
}

void Recycled() {
    Ble::StartAdvertising();
}

void ParamertersUpdated(bt_conn* conn, uint16_t interval, uint16_t latency, uint16_t timeout) {
    double connection_interval_ms = interval * 1.25;
    uint16_t supervision_timeout_ms = timeout * 10;

    LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms",
        connection_interval_ms, latency, supervision_timeout_ms);
}

// NOTE: Any logging from this callback will cause a crash,
// thus it's disabled.
void DataLengthUpdated(bt_conn* conn, bt_conn_le_data_len_info* info) {
    uint16_t tx_len = info->tx_max_len;
    uint16_t tx_time = info->tx_max_time;
    uint16_t rx_len = info->rx_max_len;
    uint16_t rx_time = info->rx_max_time;

    LOG_INF("Data length updated. Length %d/%d bytes, time %d/%d us",
        tx_len, rx_len, tx_time, rx_time);
}

void SecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) {
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if(!err) {
		LOG_INF("Security changed: %s level %u", addr, level);

        if(level >= BT_SECURITY_L2)
            k_work_reschedule(&Ble::data_length_update_work_, K_NO_WAIT);
	} else {
		LOG_INF("Security failed: %s level %u err %d", addr, level, err);
        bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
	}

    k_work_reschedule(&Ble::pairing_finished_work_, K_NO_WAIT);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = &Connected,
    .disconnected = &Disconnected,
    .recycled = &Recycled,
    .le_param_updated = &ParamertersUpdated,
    // .le_data_len_updated = &DataLengthUpdated // See note at the method implementation
    .security_changed = &SecurityChanged,
};

void Ble::ConnectedCbWorkHandler(struct k_work* work) {
    bt_conn* conn = AcquireActiveConn();
    if(conn == nullptr) {
        LOG_WRN("Connected handlers skipped: no active connection");
        return;
    }

    for(auto& [_, handler] : Ble::connected_handlers_) {
        try {
            handler(conn);
        } catch(const std::exception& e) {
            LOG_ERR("Connected handler threw: %s", e.what());
        } catch(...) {
            LOG_ERR("Connected handler threw an unknown exception");
        }
    }

    bt_conn_unref(conn);
}

void Ble::SecurityUpdateWorkHandler(struct k_work* work) {
    bt_conn* conn = AcquireActiveConn();
    if(conn == nullptr) {
        LOG_WRN("Security upgrade skipped: no active connection");
        return;
    }

    int sec_err = bt_conn_set_security(conn, BT_SECURITY_L2);
    if(sec_err)
        LOG_ERR("Failed to set security (err %d)", sec_err);
    else
        LOG_INF("Security upgrade initiated");

    bt_conn_unref(conn);
}

void Ble::DataLengthUpdateWorkHandler(struct k_work* work) {
    bt_conn* conn = AcquireActiveConn();
    if(conn == nullptr) {
        LOG_WRN("Data length update skipped: no active connection");
        return;
    }

    Ble::UpdateDataLength(conn);

    // TODO: Doesn't seem to work with iOS, investigate.
    // Ble::UpdateMtu(conn);

    bt_conn_unref(conn);
}

void Ble::PairingStartedWorkHandler(struct k_work* work) {
    for(auto& [_, handler] : Ble::pairing_started_handlers_) {
        try {
            handler();
        } catch(const std::exception& e) {
            LOG_ERR("Pairing started handler threw: %s", e.what());
        } catch(...) {
            LOG_ERR("Pairing started handler threw an unknown exception");
        }
    }
}

void Ble::PairingFinishedWorkHandler(struct k_work* work) {
    for(auto& [_, handler] : Ble::pairing_finished_handlers_) {
        try {
            handler();
        } catch(const std::exception& e) {
            LOG_ERR("Pairing finished handler threw: %s", e.what());
        } catch(...) {
            LOG_ERR("Pairing finished handler threw an unknown exception");
        }
    }
}

int Ble::RegisterConnectedHandler(ConnectedHandler handler) {
    static int id = 0;
    connected_handlers_[id] = handler;
    return id++;
}
int Ble::RegisterDisconnectedHandler(DisconnectedHandler handler) {
    static int id = 0;
    disconnected_handlers_[id] = handler;
    return id++;
}
int Ble::RegisterPairingStartedHandler(PairingStartedHandler handler) {
    static int id = 0;
    pairing_started_handlers_[id] = handler;
    return id++;
}
int Ble::RegisterPairingFinishedHandler(PairingFinishedHandler handler) {
    static int id = 0;
    pairing_finished_handlers_[id] = handler;
    return id++;
}

void Ble::UnregisterConnectedHandler(int id) {
    connected_handlers_.erase(id);
}
void Ble::UnregisterDisconnectedHandler(int id) {
    disconnected_handlers_.erase(id);
}
void Ble::UnregisterPairingStartedHandler(int id) {
    pairing_started_handlers_.erase(id);
}
void Ble::UnregisterPairingFinishedHandler(int id) {
    pairing_finished_handlers_.erase(id);
}

} // namespace eerie_leap::subsys::bluetooth

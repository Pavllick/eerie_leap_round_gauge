#include <algorithm>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include "ble.h"

LOG_MODULE_REGISTER(ble);

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

const bt_data Ble::ad_[] = {
    // Flags: general discoverable, no BR/EDR
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

    // Full device name
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

const bt_le_adv_param* Ble::advertising_params_ = BT_LE_ADV_PARAM(
    BT_LE_ADV_OPT_CONN,
    BT_GAP_ADV_FAST_INT_MIN_2,
    BT_GAP_ADV_FAST_INT_MAX_2,
    nullptr
);

bt_conn* Ble::active_conn_{nullptr};
BleCallbacks Ble::callbacks_;
k_work_delayable Ble::adv_restart_work_;

k_work_delayable Ble::connected_cb_work_;
k_work_delayable Ble::security_update_work_;
k_work_delayable Ble::data_length_update_work_;
k_work_delayable Ble::pairing_started_work_;
k_work_delayable Ble::pairing_finished_work_;

bool Ble::Initialize(BleCallbacks callbacks) {
    callbacks_ = callbacks;
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

    settings_load();

    LOG_INF("Bluetooth initialized");

    return StartAdvertising() == 0;
}

// Advertising
// ===========

int Ble::StartAdvertising() {
    return k_work_schedule(&adv_restart_work_, K_NO_WAIT);
}

void Ble::RestartAdvertisingWorkHandler(struct k_work* work) {
    bt_le_adv_stop();

    int err = bt_le_adv_start(advertising_params_, ad_, ARRAY_SIZE(ad_), nullptr, 0);
    if(err) {
        LOG_ERR("Failed to start advertising (err %d)", err);
        return;
    }

    LOG_INF("BLE advertising started");
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

    if(Ble::active_conn_)
        bt_conn_unref(Ble::active_conn_);
    Ble::active_conn_ = bt_conn_ref(conn);

    k_work_schedule(&Ble::connected_cb_work_, K_NO_WAIT);
    k_work_schedule(&Ble::pairing_started_work_, K_NO_WAIT);
    k_work_schedule(&Ble::security_update_work_, K_NO_WAIT);
}

void Disconnected(bt_conn* conn, uint8_t reason) {
    LOG_INF("Disconnected (reason %u)", reason);

    if(Ble::callbacks_.disconnected)
        Ble::callbacks_.disconnected(conn);

    if(Ble::active_conn_)
        bt_conn_unref(Ble::active_conn_);
    Ble::active_conn_ = nullptr;
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
            k_work_schedule(&Ble::data_length_update_work_, K_NO_WAIT);
	} else {
		LOG_INF("Security failed: %s level %u err %d", addr, level, err);
        bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
	}

    k_work_schedule(&Ble::pairing_finished_work_, K_NO_WAIT);
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
    if(callbacks_.connected)
        callbacks_.connected(active_conn_);
}

void Ble::SecurityUpdateWorkHandler(struct k_work* work) {
    int sec_err = bt_conn_set_security(active_conn_, BT_SECURITY_L2);
    if(sec_err)
        LOG_ERR("Failed to set security (err %d)", sec_err);
    else
        LOG_INF("Security upgrade initiated");
}

void Ble::DataLengthUpdateWorkHandler(struct k_work* work) {
    Ble::UpdateDataLength(Ble::active_conn_);

    // TODO: Doesn't seem to work with iOS, investigate.
    // Ble::UpdateMtu(Ble::active_conn_);
}

void Ble::PairingStartedWorkHandler(struct k_work* work) {
    if(Ble::callbacks_.pairing_started)
        Ble::callbacks_.pairing_started();
}

void Ble::PairingFinishedWorkHandler(struct k_work* work) {
    if(Ble::callbacks_.pairing_finished)
        Ble::callbacks_.pairing_finished();
}


} // namespace eerie_leap::subsys::bluetooth

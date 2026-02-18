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

namespace eerie_leap::subsys::bluetooth {

struct BleCallbacks {
    using ConnectedHandler = std::function<void(bt_conn* conn)>;
    using DisconnectedHandler = std::function<void(bt_conn* conn)>;
    using PairingStartedHandler = std::function<void()>;
    using PairingFinishedHandler = std::function<void()>;

    ConnectedHandler connected;
    DisconnectedHandler disconnected;
    PairingStartedHandler pairing_started;
    PairingFinishedHandler pairing_finished;
};

class Ble {
private:
    static const bt_data ad_[];
    static const bt_le_adv_param* advertising_params_;
    static bt_conn* active_conn_;
    static BleCallbacks callbacks_;

    static k_work_delayable adv_restart_work_;
    static k_work_delayable connected_cb_work_;
    static k_work_delayable security_update_work_;
    static k_work_delayable data_length_update_work_;
    static k_work_delayable pairing_started_work_;
    static k_work_delayable pairing_finished_work_;

    Ble() = default;
    ~Ble() = default;

    Ble(const Ble&) = delete;
    Ble& operator=(const Ble&) = delete;

    static int StartAdvertising();

    friend void Connected(bt_conn* conn, uint8_t err);
    friend void Disconnected(bt_conn* conn, uint8_t reason);
    friend void Recycled();
    friend void ParamertersUpdated(bt_conn* conn, uint16_t interval, uint16_t latency, uint16_t timeout);
    friend void DataLengthUpdated(bt_conn* conn, bt_conn_le_data_len_info* info);
    friend void SecurityChanged(bt_conn* conn, bt_security_t level, enum bt_security_err err);

    friend void PairingFailed(struct bt_conn *conn, enum bt_security_err reason);

    static void UpdateDataLength(bt_conn* conn);
    static void UpdateMtu(bt_conn* conn);
    static void GattExchangeParamsFunc(bt_conn* conn, uint8_t att_err, bt_gatt_exchange_params* params);
    static void RestartAdvertisingWorkHandler(struct k_work* work);
    static void ConnectedCbWorkHandler(struct k_work* work);
    static void SecurityUpdateWorkHandler(struct k_work* work);
    static void DataLengthUpdateWorkHandler(struct k_work* work);
    static void PairingStartedWorkHandler(struct k_work* work);
    static void PairingFinishedWorkHandler(struct k_work* work);

public:
    static bool Initialize(BleCallbacks callbacks);
};

} // namespace eerie_leap::subsys::bluetooth

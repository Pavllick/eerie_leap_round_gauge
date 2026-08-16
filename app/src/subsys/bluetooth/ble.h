#pragma once

#include <span>
#include <functional>
#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory_resource>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/kernel.h>

#include "./utilities/ad_buffer.hpp"

namespace eerie_leap::subsys::bluetooth {

using eerie_leap::subsys::bluetooth::utilities::AdBuffer;

class Ble {
public:
    using ConnectedHandler = std::function<void(bt_conn* conn)>;
    using DisconnectedHandler = std::function<void(bt_conn* conn)>;
    using PairingStartedHandler = std::function<void()>;
    using PairingFinishedHandler = std::function<void()>;

private:
    static AdBuffer ad_;
    static AdBuffer sd_;
    static const bt_le_adv_param* advertising_params_;

    // Guards active_conn_: it is written from the BT RX workqueue (conn callbacks)
    // and read from the system workqueue (deferred handlers below).
    static k_mutex conn_mutex_;
    static bt_conn* active_conn_;

    static k_work_delayable adv_restart_work_;
    static k_work_delayable connected_cb_work_;
    static k_work_delayable security_update_work_;
    static k_work_delayable data_length_update_work_;
    static k_work_delayable pairing_started_work_;
    static k_work_delayable pairing_finished_work_;

    static std::unordered_map<int, ConnectedHandler> connected_handlers_;
    static std::unordered_map<int, DisconnectedHandler> disconnected_handlers_;
    static std::unordered_map<int, PairingStartedHandler> pairing_started_handlers_;
    static std::unordered_map<int, PairingFinishedHandler> pairing_finished_handlers_;

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

    // Returns a new reference to the active connection, or nullptr. Caller must bt_conn_unref().
    static bt_conn* AcquireActiveConn();

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
    static bool Initialize();
    static bool Start();

    static void UpdateAdvertisingData(AdBuffer&& ad);
    static void UpdateScanResponseData(AdBuffer&& sd);

    static int RegisterConnectedHandler(ConnectedHandler handler);
    static int RegisterDisconnectedHandler(DisconnectedHandler handler);
    static int RegisterPairingStartedHandler(PairingStartedHandler handler);
    static int RegisterPairingFinishedHandler(PairingFinishedHandler handler);

    static void UnregisterConnectedHandler(int id);
    static void UnregisterDisconnectedHandler(int id);
    static void UnregisterPairingStartedHandler(int id);
    static void UnregisterPairingFinishedHandler(int id);
};

} // namespace eerie_leap::subsys::bluetooth

// #pragma once

// #include <zephyr/modbus/modbus.h>

// #include "modbus_callbacks.h"

// namespace eerie_leap::subsys::modbus {

// class Modbus {
// private:
//     const char* iface_name_;
//     modbus_iface_param client_params_;
//     int client_iface_;
//     modbus_user_callbacks callbacks_;
//     ModbusCallbacks callbacks_ext_;

// public:
//     static const uint8_t DEFAULT_SERVER_ID = 0xFF;

//     explicit Modbus(const char* iface_name, uint8_t server_id = DEFAULT_SERVER_ID);

//     int Initialize(ModbusCallbacks callbacks_ext);
//     int UpdateServerId(uint8_t server_id);

//     static int ReadRegistersCallback(uint16_t addr, uint16_t *reg, uint16_t num_regs);
//     static int WriteRegisterCallback(uint16_t addr, uint16_t reg);
//     static int WriteRegistersCallback(uint16_t addr, uint16_t *reg, uint16_t num_regs);

//     uint8_t GetServerId() const { return client_params_.server.unit_id; }
// };

// }  // namespace eerie_leap::subsys::modbus

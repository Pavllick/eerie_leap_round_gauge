#include <zephyr/logging/log.h>

#include "modbus.h"

LOG_MODULE_REGISTER(modbus_logger);

namespace eerie_leap::subsys::modbus {

// TODO: Come up with a better way to handle this
// callbacks have to be static, but we need to access instance members
Modbus* instance = nullptr;

Modbus::Modbus(const char* iface_name, uint8_t server_id) : iface_name_(iface_name), client_iface_(modbus_iface_get_by_name(iface_name_)) {
    callbacks_ = {
        .holding_regs_rd = ReadRegistersCallback,
        .holding_reg_wr = WriteRegisterCallback,
        .holding_regs_wr = WriteRegistersCallback,
    };

    client_params_ = {
        .mode = MODBUS_MODE_RTU,
        .server = {
            .user_cb = &callbacks_,
            .unit_id = server_id == 0 ? DEFAULT_SERVER_ID : server_id,
        },
        .serial = {
            .baud = CONFIG_EERIE_LEAP_MODBUS_BAUD_RATE,
            .parity = UART_CFG_PARITY_NONE,
        },
    };

    LOG_INF("Modbus initialized, server ID: %d", server_id);

    instance = this;
}

int Modbus::Initialize(ModbusCallbacks callbacks_ext) {
    callbacks_ext_ = callbacks_ext;
    return modbus_init_server(client_iface_, client_params_);
};

int Modbus::UpdateServerId(uint8_t server_id) {
    LOG_DBG("Updating server ID to %d", server_id);

    modbus_disable(client_iface_);
    client_params_.server.unit_id = server_id;

    return modbus_init_server(client_iface_, client_params_);
}

int Modbus::ReadRegistersCallback(uint16_t addr, uint16_t *reg, uint16_t num_regs) {
	if (instance->callbacks_ext_.holding_regs_rd) {
		return instance->callbacks_ext_.holding_regs_rd(addr, reg, num_regs);
	}

	return MODBUS_EXC_ILLEGAL_FC;
}

int Modbus::WriteRegisterCallback(uint16_t addr, uint16_t reg) {
	if (instance->callbacks_ext_.holding_reg_wr) {
		return instance->callbacks_ext_.holding_reg_wr(addr, reg);
	}

	return MODBUS_EXC_ILLEGAL_FC;
}

int Modbus::WriteRegistersCallback(uint16_t addr, uint16_t* reg, uint16_t num_regs) {
	if (instance->callbacks_ext_.holding_regs_wr) {
		return instance->callbacks_ext_.holding_regs_wr(addr, reg, num_regs);
	}

	return MODBUS_EXC_ILLEGAL_FC;
}

int Modbus::ReadRegisters(uint8_t address, uint16_t start_address, void* data, size_t count) {
    return modbus_read_holding_regs(client_iface_, address, start_address, (uint16_t*)data, count);
}

int Modbus::WriteRegisters(uint8_t address, uint16_t start_address, void* data, size_t count) {
    return modbus_write_holding_regs(client_iface_, address, start_address, (uint16_t*)data, count);
}

}  // namespace eerie_leap::subsys::modbus

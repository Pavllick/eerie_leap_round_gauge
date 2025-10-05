#include <zephyr/logging/log.h>

#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "types/sensor_reading_dto.h"

#include "interface.h"

namespace eerie_leap::domain::interface_domain {

using namespace eerie_leap::domain::ui_domain::event_bus;

LOG_MODULE_REGISTER(interface_logger);

Interface::Interface(ext_unique_ptr<Modbus> modbus, std::shared_ptr<SystemConfigurationManager> system_configuration_manager)
    : modbus_(std::move(modbus)),
    system_configuration_manager_(std::move(system_configuration_manager)),
    server_id_(modbus_->GetServerId()),
    server_id_counter_(0) {

    server_id_resolved_ = server_id_ != Modbus::DEFAULT_SERVER_ID;
    status_ = ComUserStatus::NOT_CONFIGURED;
}

int Interface::Initialize() {
    device_id_ = system_configuration_manager_->Get()->device_id;

    ModbusCallbacks callbacks = {
        .holding_regs_rd = [this](uint16_t addr, uint16_t *reg, uint16_t num_regs) -> int {
            return this->ReadHoldingRegisters(addr, reg, num_regs);
        },

        .holding_reg_wr = [this](uint16_t addr, const uint16_t& reg) -> int {
            return this->WriteHoldingRegister(addr, reg);
        },

        .holding_regs_wr = [this](uint16_t addr, const uint16_t* reg, uint16_t num_regs) -> int {
            return this->WriteHoldingRegisters(addr, reg, num_regs);
        },
    };

    return modbus_->Initialize(callbacks);
}

void Interface::SetStatus(ComUserStatus status) {
    status_ = status;
}

ComUserStatus Interface::GetStatus() const {
    return status_;
}

int Interface::Get(ComRequestType com_request_type, uint16_t* data, size_t size_bytes) {
    if(com_request_type == ComRequestType::GET_RESOLVE_SERVER_ID_GUID) {
        for(int i = 0; i < size_bytes / 2; i++) {
            data[i] = ((uint16_t*)&device_id_)[i];
        }

        return 0;
    } else if(com_request_type == ComRequestType::GET_STATUS) {
        for(int i = 0; i < size_bytes / 2; i++) {
            data[0] = (uint16_t)status_;
        }

        return 0;
    }

    return -1;
}

int Interface::Set(ComRequestType com_request_type, const uint16_t* data, size_t size_bytes) {
    if(com_request_type == ComRequestType::SET_RESOLVE_SERVER_ID) {
        server_id_resolved_ = false;
        server_id_counter_ = 0;

        return ServerIdResolveNext();
    } else if(com_request_type == ComRequestType::SET_RESOLVE_SERVER_ID_GUID) {
        uint64_t device_id = *(uint64_t*)data;

        if(device_id == device_id_) {
            if(!system_configuration_manager_->UpdateInterfaceChannel(modbus_->GetServerId()))
                return -1;

            server_id_resolved_ = true;
            LOG_DBG("Server ID resolved, ID: %d", modbus_->GetServerId());

            return 0;
        }

        return ServerIdResolveNext();
    } else if(com_request_type == ComRequestType::SET_READING) {
        SensorReadingDto reading = *(SensorReadingDto*)data;
        Process<SensorReadingDto>(&reading);

        return 0;
    } else if(com_request_type == ComRequestType::SET_ACK) {
        ComUserStatus ack_status = *(ComUserStatus*)data;

        if(ack_status == status_)
            status_ = ComUserStatus::OK;

        return 0;
    } else if(com_request_type == ComRequestType::SET_STATUS_UPDATE_OK) {
        ComUserStatus status = *(ComUserStatus*)data;
        UserStatus user_status {
            .status = *(ComUserStatus*)data,
            .is_ok = true
        };

        Process<UserStatus>(&user_status);

        return 0;
    } else if(com_request_type == ComRequestType::SET_STATUS_UPDATE_FAIL) {
        ComUserStatus status = *(ComUserStatus*)data;
        UserStatus user_status {
            .status = *(ComUserStatus*)data,
            .is_ok = false
        };

        Process<UserStatus>(&user_status);

        return 0;
    }

    return -1;
}

template<typename T>
int Interface::Process(T* value) {
    auto it = processors_.find(std::type_index(typeid(T)));
    if (it != processors_.end())
        return it->second(value);

    return -1;
}

int Interface::ReadHoldingRegister(uint16_t addr, uint16_t *reg) {
    return Get(static_cast<ComRequestType>(addr), reg, sizeof(uint16_t));
}

int Interface::ReadHoldingRegisters(uint16_t addr, uint16_t *reg, uint16_t num_regs) {
    return Get(static_cast<ComRequestType>(addr), reg, num_regs * sizeof(uint16_t));
}

int Interface::WriteHoldingRegister(uint16_t addr, const uint16_t& reg) {
    return Set(static_cast<ComRequestType>(addr), &reg, sizeof(uint16_t));
}

int Interface::WriteHoldingRegisters(uint16_t addr, const uint16_t* reg, uint16_t num_regs) {
    return Set(static_cast<ComRequestType>(addr), reg, num_regs * sizeof(uint16_t));
}

int Interface::ServerIdResolveNext() {
    server_id_counter_++;
    LOG_INF("Server ID attempted to resolve: %d", server_id_counter_);

    modbus_->UpdateServerId(server_id_counter_);

    return 0;
}

} // namespace eerie_leap::domain::interface_domain

#include <zephyr/logging/log.h>

#include "domain/sensor_domain/services/reading_processor_service.h"

#include "types/sensor_reading_dto.h"
#include "interface.h"

namespace eerie_leap::domain::interface_domain {

LOG_MODULE_REGISTER(interface_logger);

Interface::Interface(std::shared_ptr<Modbus> modbus, std::shared_ptr<GuidGenerator> guid_generator, std::shared_ptr<ReadingProcessorService> reading_processor_service)
    : modbus_(modbus),
    guid_generator_(std::move(guid_generator)),
    reading_processor_service_(std::move(reading_processor_service)),
    server_id_(modbus->GetServerId()),
    server_id_counter_(0) {

    server_id_resolved_ = server_id_ != Modbus::DEFAULT_SERVER_ID;
}

int Interface::Initialize() {
    server_guid_ = guid_generator_->Generate();
    LOG_INF("Interface Server GUID: %llu", server_guid_.AsUint64());

    ModbusCallbacks callbacks = {
        .holding_regs_rd = [this](uint16_t addr, uint16_t *reg, uint16_t num_regs) -> int {
            return this->ReadHoldingRegisters(addr, reg, num_regs);
        },

        .holding_reg_wr = [this](uint16_t addr, uint16_t reg) -> int {
            return this->WriteHoldingRegister(addr, reg);
        },

        .holding_regs_wr = [this](uint16_t addr, uint16_t* reg, uint16_t num_regs) -> int {
            return this->WriteHoldingRegisters(addr, reg, num_regs);
        },
    };

    return modbus_->Initialize(callbacks);
}

int Interface::Get(RequestType request_type, uint16_t* data, size_t size_bytes) {
    if(request_type == RequestType::GET_RESOLVE_SERVER_ID_GUID) {
        for(int i = 0; i < size_bytes; i++) {
            data[i] = ((uint16_t*)&server_guid_)[i];
        }

        return 0;
    }

    return -1;
}

int Interface::Set(RequestType request_type, uint16_t* data, size_t size_bytes) {
    if(request_type == RequestType::GET_RESOLVE_SERVER_ID) {
        server_id_resolved_ = false;
        server_id_counter_ = 0;

        return ServerIdResolveNext();
    } else if(request_type == RequestType::SET_RESOLVE_SERVER_ID_GUID) {
        Guid guid = *(Guid*)data;

        if(guid.AsUint64() == server_guid_.AsUint64()) {
            server_id_resolved_ = true;
            LOG_DBG("Server ID resolved, ID: %d", modbus_->GetServerId());

            return 0;
        }

        return ServerIdResolveNext();
    } else if(request_type == RequestType::SET_READING) {
        SensorReadingDto reading = *(SensorReadingDto*)data;
        reading_processor_service_->ProcessReading(reading);

        return 0;
    }

    return -1;
}

int Interface::ReadHoldingRegister(uint16_t addr, uint16_t *reg) {
    return Get(static_cast<RequestType>(addr), reg, sizeof(uint16_t));
}

int Interface::ReadHoldingRegisters(uint16_t addr, uint16_t *reg, uint16_t num_regs) {
    return Get(static_cast<RequestType>(addr), reg, num_regs * sizeof(uint16_t));
}

int Interface::WriteHoldingRegister(uint16_t addr, uint16_t reg) {
    return Set(static_cast<RequestType>(addr), &reg, sizeof(uint16_t));
}

int Interface::WriteHoldingRegisters(uint16_t addr, uint16_t* reg, uint16_t num_regs) {
    return Set(static_cast<RequestType>(addr), reg, num_regs * sizeof(uint16_t));
}

int Interface::ServerIdResolveNext() {
    server_id_counter_++;
    LOG_INF("Server ID attempted to resolve: %d", server_id_counter_);

    modbus_->UpdateServerId(server_id_counter_);

    return 0;
}

} // namespace eerie_leap::domain::interface_domain

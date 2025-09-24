#pragma once

#include <cstdint>
#include <memory>

#include "subsys/modbus/modbus.h"
#include "domain/sensor_domain/services/reading_processor_service.h"
#include "controllers/configuration/system_configuration_controller.h"

#include "types/com_request_type.h"
#include "types/com_user_status.h"

namespace eerie_leap::domain::interface_domain {

using namespace eerie_leap::subsys::modbus;
using namespace eerie_leap::domain::sensor_domain::services;
using namespace eerie_leap::domain::interface_domain::types;
using namespace eerie_leap::controllers::configuration;

class Interface {
private:
    std::shared_ptr<Modbus> modbus_;
    std::shared_ptr<SystemConfigurationController> system_configuration_controller_;
    std::shared_ptr<ReadingProcessorService> reading_processor_service_;

    uint8_t server_id_;
    uint8_t server_id_counter_;
    bool server_id_resolved_;
    uint64_t device_id_;
    ComUserStatus status_;

    int Get(ComRequestType com_request_type, uint16_t* data, size_t size_bytes);
    int Set(ComRequestType com_request_type, uint16_t* data, size_t size_bytes);

    int ReadHoldingRegister(uint16_t addr, uint16_t *reg);
    int ReadHoldingRegisters(uint16_t addr, uint16_t *reg, uint16_t num_regs);
    int WriteHoldingRegister(uint16_t addr, uint16_t reg);
    int WriteHoldingRegisters(uint16_t addr, uint16_t* reg, uint16_t num_regs);

    int ServerIdResolveNext();

public:
    Interface(std::shared_ptr<Modbus> modbus,
        std::shared_ptr<SystemConfigurationController> system_configuration_controller,
        std::shared_ptr<ReadingProcessorService> reading_processor_service);
    int Initialize();

    void SetStatus(ComUserStatus status);
    ComUserStatus GetStatus() const;
};

} // namespace eerie_leap::domain::interface_domain

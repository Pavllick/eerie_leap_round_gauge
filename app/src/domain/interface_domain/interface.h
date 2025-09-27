#pragma once

#include <cstdint>
#include <memory>

#include "subsys/modbus/modbus.h"
#include "domain/sensor_domain/services/reading_processor_service.h"
#include "domain/system_domain/configuration/system_configuration_manager.h"

#include "types/com_request_type.h"
#include "types/com_user_status.h"
#include "interface_status_update_handler.h"

namespace eerie_leap::domain::interface_domain {

using namespace eerie_leap::subsys::modbus;
using namespace eerie_leap::domain::sensor_domain::services;
using namespace eerie_leap::domain::interface_domain::types;
using namespace eerie_leap::domain::system_domain::configuration;

class Interface {
private:
    std::shared_ptr<Modbus> modbus_;
    std::shared_ptr<SystemConfigurationManager> system_configuration_manager_;
    std::shared_ptr<ReadingProcessorService> reading_processor_service_;

    uint8_t server_id_;
    uint8_t server_id_counter_;
    bool server_id_resolved_;
    uint64_t device_id_;
    ComUserStatus status_;
    std::shared_ptr<std::unordered_map<ComUserStatus, std::shared_ptr<std::vector<InterfaceStatusUpdateHandler>>>> status_update_handlers_;

    int Get(ComRequestType com_request_type, uint16_t* data, size_t size_bytes);
    int Set(ComRequestType com_request_type, uint16_t* data, size_t size_bytes);

    int ReadHoldingRegister(uint16_t addr, uint16_t *reg);
    int ReadHoldingRegisters(uint16_t addr, uint16_t *reg, uint16_t num_regs);
    int WriteHoldingRegister(uint16_t addr, uint16_t reg);
    int WriteHoldingRegisters(uint16_t addr, uint16_t* reg, uint16_t num_regs);

    int ServerIdResolveNext();

public:
    Interface(std::shared_ptr<Modbus> modbus,
        std::shared_ptr<SystemConfigurationManager> system_configuration_manager,
        std::shared_ptr<ReadingProcessorService> reading_processor_service);
    int Initialize();

    void SetStatus(ComUserStatus status);
    ComUserStatus GetStatus() const;
    void RegisterStatusUpdateHandler(ComUserStatus status, InterfaceStatusUpdateHandler handler);
};

} // namespace eerie_leap::domain::interface_domain

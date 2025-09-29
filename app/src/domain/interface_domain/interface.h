#pragma once

#include <cstdint>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <functional>
#include <unordered_map>

#include "subsys/modbus/modbus.h"
#include "domain/system_domain/configuration/system_configuration_manager.h"
#include "domain/interface_domain/processors/reading_processor.h"

#include "types/com_request_type.h"
#include "types/user_status.h"
#include "processors/i_interface_processor.h"

namespace eerie_leap::domain::interface_domain {

using namespace eerie_leap::subsys::modbus;
using namespace eerie_leap::domain::interface_domain::types;
using namespace eerie_leap::domain::system_domain::configuration;
using namespace eerie_leap::domain::interface_domain::processors;

class Interface {
private:
    std::shared_ptr<Modbus> modbus_;
    std::shared_ptr<SystemConfigurationManager> system_configuration_manager_;
    std::shared_ptr<ReadingProcessor> reading_processor_;
    std::unordered_map<std::type_index, std::function<int(void*)>> processors_;

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

    template<typename T>
    int Process(T* value);

public:
    Interface(std::shared_ptr<Modbus> modbus,
        std::shared_ptr<SystemConfigurationManager> system_configuration_manager,
        std::shared_ptr<ReadingProcessor> reading_processor);
    int Initialize();

    void SetStatus(ComUserStatus status);
    ComUserStatus GetStatus() const;

    template<typename T>
    int RegisterProcessor(std::shared_ptr<IInterfaceProcessor<T>> processor) {
        processors_[std::type_index(typeid(T))] =
            [processor](void* value_ptr) -> int {
                return processor->Process(*static_cast<T*>(value_ptr));
            };

        return 0;
    }
};

} // namespace eerie_leap::domain::interface_domain

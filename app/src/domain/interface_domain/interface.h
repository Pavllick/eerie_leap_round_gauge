#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <unordered_map>

#include "utilities/guid/guid_generator.h"
#include "subsys/modbus/modbus.h"
#include "subsys/modbus/modbus_callbacks.h"

#include "types/sensor_reading_dto.h"
#include "types/request_type.h"

namespace eerie_leap::domain::interface_domain {

using namespace eerie_leap::utilities::guid;
using namespace eerie_leap::subsys::modbus;
using namespace eerie_leap::domain::interface_domain::types;

using ReadingHandler = std::function<int(SensorReadingDto&)>;

class Interface {
private:
    std::shared_ptr<Modbus> modbus_;
    std::shared_ptr<GuidGenerator> guid_generator_;

    uint8_t server_id_;
    uint8_t server_id_counter_;
    bool server_id_resolved_;
    Guid server_guid_;

    std::unordered_map<size_t, ReadingHandler> reading_handlers_;

    static const uint16_t RESPONSE_SUCCESS = 0;
    static const uint16_t RESPONSE_FAILURE = 1;

    int Get(RequestType request_type, uint16_t* data, size_t size_bytes);
    int Set(RequestType request_type, uint16_t* data, size_t size_bytes);

    int ReadHoldingRegister(uint16_t addr, uint16_t *reg);
    int ReadHoldingRegisters(uint16_t addr, uint16_t *reg, uint16_t num_regs);
    int WriteHoldingRegister(uint16_t addr, uint16_t reg);
    int WriteHoldingRegisters(uint16_t addr, uint16_t* reg, uint16_t num_regs);

    int ServerIdResolveNext();

public:
    explicit Interface(std::shared_ptr<Modbus> modbus, std::shared_ptr<GuidGenerator> guid_generator);
    int Initialize();

    int RegisterReadingHandler(size_t sensor_id_hash, ReadingHandler handler);
};

} // namespace eerie_leap::domain::interface_domain

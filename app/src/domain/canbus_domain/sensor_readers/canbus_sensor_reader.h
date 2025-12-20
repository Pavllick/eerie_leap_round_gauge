#pragma once

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <chrono>

#include <zephyr/kernel.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/atomic.h>

#include "utilities/memory/memory_resource_manager.h"
#include "utilities/guid/guid_generator.h"
#include "subsys/time/i_time_service.h"
#include "subsys/canbus/canbus.h"
#include "subsys/dbc/dbc.h"
#include "domain/canbus_domain/processors/reading_processor.h"

namespace eerie_leap::domain::canbus_domain::sensor_readers {

using namespace std::chrono;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::guid;
using namespace eerie_leap::subsys::time;
using namespace eerie_leap::subsys::canbus;
using namespace eerie_leap::subsys::dbc;
using namespace eerie_leap::domain::canbus_domain::processors;

class CanbusSensorReader {
protected:
    std::shared_ptr<ITimeService> time_service_;
    std::shared_ptr<GuidGenerator> guid_generator_;
    std::shared_ptr<Canbus> canbus_;
    std::shared_ptr<Dbc> dbc_;

    k_spinlock can_frame_lock_;
    ReadingProcessor reading_processor_;
    std::unordered_map<uint32_t, std::unordered_set<size_t>> frames_signals_map_;

    void Process(const CanFrame& frame);

public:
    CanbusSensorReader(
        std::shared_ptr<ITimeService> time_service,
        std::shared_ptr<GuidGenerator> guid_generator,
        std::shared_ptr<Canbus> canbus,
        std::shared_ptr<Dbc> dbc);
    virtual ~CanbusSensorReader() = default;
};

} // namespace eerie_leap::domain::canbus_domain::sensor_readers

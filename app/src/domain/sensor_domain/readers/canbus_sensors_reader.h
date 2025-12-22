#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <chrono>

#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include "utilities/memory/memory_resource_manager.h"

#include "subsys/time/i_time_service.h"
#include "subsys/canbus/canbus.h"
#include "subsys/dbc/dbc.h"

#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"

namespace eerie_leap::domain::sensor_domain::readers {

using namespace std::chrono;
using namespace eerie_leap::utilities::memory;

using namespace eerie_leap::subsys::time;
using namespace eerie_leap::subsys::canbus;
using namespace eerie_leap::subsys::dbc;

using namespace eerie_leap::domain::sensor_domain::utilities;

class CanbusSensorsReader {
protected:
    std::shared_ptr<ITimeService> time_service_;
    std::shared_ptr<Canbus> canbus_;
    std::shared_ptr<Dbc> dbc_;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame_;

    std::unordered_map<uint32_t, std::vector<int>> registered_handler_ids_;

    std::unordered_map<uint32_t, std::unordered_set<size_t>> frames_signals_map_;

    void UpdateReadings(const CanFrame& frame);

public:
    CanbusSensorsReader(
        std::shared_ptr<ITimeService> time_service,
        std::shared_ptr<Canbus> canbus,
        std::shared_ptr<Dbc> dbc,
        std::shared_ptr<SensorReadingsFrame> sensor_readings_frame);
    virtual ~CanbusSensorsReader();
};

} // namespace eerie_leap::domain::sensor_domain::readers

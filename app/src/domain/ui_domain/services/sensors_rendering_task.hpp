#pragma once

#include <memory>
#include <zephyr/kernel.h>

#include "subsys/threading/work_queue_thread.h"
#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::subsys::threading;
using namespace eerie_leap::domain::sensor_domain::utilities;

struct SensorsRenderingTask {
    k_timeout_t refresh_rate_ms;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame;
};

} // namespace eerie_leap::domain::ui_domain::services

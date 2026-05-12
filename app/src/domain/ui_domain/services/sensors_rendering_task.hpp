#pragma once

#include <memory>
#include <zephyr/kernel.h>

#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"

namespace eerie_leap::domain::ui_domain::services {

using eerie_leap::domain::sensor_domain::utilities::SensorReadingsFrame;

struct SensorsRenderingTask {
    k_timeout_t refresh_rate_ms;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame;
};

} // namespace eerie_leap::domain::ui_domain::services

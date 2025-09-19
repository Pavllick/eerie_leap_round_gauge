#pragma once

#include <vector>

#include <zephyr/kernel.h>

#include "domain/interface_domain/types/sensor_reading_dto.h"
#include "reading_handler.h"

namespace eerie_leap::domain::sensor_domain::services {

using namespace eerie_leap::domain::interface_domain::types;

struct ReadingTask {
    k_work work;
    k_sem* processing_semaphore;
    std::vector<ReadingHandler> reading_handlers;
    SensorReadingDto reading;
};

} // namespace eerie_leap::domain::sensor_domain::services

#pragma once

#include <chrono>

#include "utilities/guid/guid.hpp"
#include "domain/sensor_domain/models/reading_status.h"

namespace eerie_leap::domain::interface_domain::types {

using namespace eerie_leap::utilities::guid;
using namespace eerie_leap::domain::sensor_domain::models;

struct SensorReadingDto {
    Guid id;
    size_t sensor_id_hash;
    std::chrono::milliseconds timestamp_ms;
    float value;
    ReadingStatus status;
};

} // namespace eerie_leap::domain::interface_domain::types

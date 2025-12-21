#pragma once

#include <chrono>

#include "utilities/guid/guid.hpp"

namespace eerie_leap::domain::sensor_domain::models {

using namespace eerie_leap::utilities::guid;

struct SensorReadingDto {
    size_t id_hash;
    std::chrono::milliseconds timestamp_ms;
    float value;
};

} // namespace eerie_leap::domain::sensor_domain::models

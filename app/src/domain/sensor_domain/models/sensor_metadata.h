#pragma once

#include <string>

namespace eerie_leap::domain::sensor_domain::models {

struct SensorMetadata {
    std::string name;
    std::string unit;
    std::string description;
};

} // namespace eerie_leap::domain::sensor_domain::models

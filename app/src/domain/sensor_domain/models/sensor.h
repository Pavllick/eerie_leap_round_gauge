#pragma once

#include <string>

#include "sensor_metadata.h"
#include "sensor_configuration.h"

namespace eerie_leap::domain::sensor_domain::models {

struct Sensor {
    std::string id;
    size_t id_hash;
    SensorMetadata metadata;
    SensorConfiguration configuration;
};

} // namespace eerie_leap::domain::sensor_domain::models

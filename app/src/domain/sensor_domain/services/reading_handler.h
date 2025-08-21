#pragma once

#include <functional>

#include "domain/interface_domain/types/sensor_reading_dto.h"

namespace eerie_leap::domain::sensor_domain::services {

using namespace eerie_leap::domain::interface_domain::types;

using ReadingHandler = std::function<int(SensorReadingDto&)>;

} // namespace eerie_leap::domain::sensor_domain::services

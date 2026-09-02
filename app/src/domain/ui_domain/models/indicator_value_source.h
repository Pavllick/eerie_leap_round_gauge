#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

// Persisted as an int in widget configuration (VALUE_SOURCE) - append only.
// Sensor is the default, so configurations that only set SENSOR_ID keep working.
enum class IndicatorValueSource : std::uint32_t {
    Sensor = 0,
    UiSignal = 1
};

} // namespace eerie_leap::domain::ui_domain::models

#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

namespace eerie_leap::domain::sensor_domain::models {

using namespace std::string_view_literals;

enum class SensorType : std::uint8_t {
    PHYSICAL_ANALOG,
    VIRTUAL_ANALOG,
    PHYSICAL_INDICATOR,
    VIRTUAL_INDICATOR
};

constexpr const std::array SensorTypeNames = {
    "PHYSICAL_ANALOG"sv,
    "VIRTUAL_ANALOG"sv,
    "PHYSICAL_INDICATOR"sv,
    "VIRTUAL_INDICATOR"sv
};


inline const char* GetSensorTypeName(SensorType type) {
    return SensorTypeNames[static_cast<std::uint8_t>(type)].data();
}

inline SensorType GetSensorType(const std::string& name) {
    for(size_t i = 0; i < size(SensorTypeNames); ++i)
        if(SensorTypeNames[i] == name)
            return static_cast<SensorType>(i);

    throw std::runtime_error("Invalid sensor type.");
}

} // namespace eerie_leap::domain::sensor_domain::models

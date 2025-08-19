#pragma once

#include <cstdint>

namespace eerie_leap::domain::sensor_domain::models {

enum class ReadingStatus : uint8_t {
    UNINITIALIZED,
    RAW,
    PROCESSED,
    CALIBRATION,
    ERROR,
};

}

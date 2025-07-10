#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

#include "interpolation_method.h"

namespace eerie_leap::utilities::voltage_interpolator {

using namespace std::string_view_literals;

enum class InterpolationMethod : std::uint8_t {
    NONE,
    LINEAR,
    CUBIC_SPLINE
};

constexpr const std::array InterpolationMethodNames = {
    "NONE"sv,
    "LINEAR"sv,
    "CUBIC_SPLINE"sv
};

inline const char* GetInterpolationMethodName(InterpolationMethod method) {
    return InterpolationMethodNames[static_cast<std::uint8_t>(method)].data();
}

inline InterpolationMethod GetInterpolationMethod(const std::string& name) {
    for(size_t i = 0; i < size(InterpolationMethodNames); ++i)
        if(InterpolationMethodNames[i] == name)
            return static_cast<InterpolationMethod>(i);

    throw std::runtime_error("Invalid interpolation method type.");
}

} // namespace eerie_leap::utilities::voltage_interpolator

#pragma once

#include <memory>
#include <optional>

// #include "utilities/math_parser/expression_evaluator.h"
#include "utilities/voltage_interpolator/i_voltage_interpolator.h"
#include "sensor_type.h"

namespace eerie_leap::domain::sensor_domain::models {

// using namespace eerie_leap::utilities::math_parser;
using namespace eerie_leap::utilities::voltage_interpolator;

struct SensorConfiguration {
    SensorType type;
    std::optional<uint32_t> channel = std::nullopt;
    uint32_t sampling_rate_ms = 100;

    std::shared_ptr<IVoltageInterpolator> voltage_interpolator = nullptr;
    // std::shared_ptr<ExpressionEvaluator> expression_evaluator = nullptr;
};

} // namespace eerie_leap::domain::sensor_domain::models

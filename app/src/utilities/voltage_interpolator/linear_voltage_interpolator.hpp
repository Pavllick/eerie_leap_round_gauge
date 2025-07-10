#pragma once

#include <memory>
#include <vector>
#include <algorithm>

#include "i_voltage_interpolator.h"
#include "calibration_data.h"
#include "interpolation_method.h"

namespace eerie_leap::utilities::voltage_interpolator {

class LinearVoltageInterpolator : public IVoltageInterpolator {
private:
    static const InterpolationMethod INTERPOLATION_METHOD = InterpolationMethod::LINEAR;
    std::shared_ptr<std::vector<CalibrationData>> calibration_table_;

public:
    explicit LinearVoltageInterpolator(const std::shared_ptr<std::vector<CalibrationData>>& calibration_table)
        : calibration_table_(std::move(calibration_table)) {

        if(!calibration_table_ || calibration_table_->size() < 2)
            throw std::invalid_argument("Calibration data is missing or invalid.");
    }

    float Interpolate(float voltage, bool clamp_to_ends = false) const override {
        const auto& table = *calibration_table_;

        if(clamp_to_ends) {
            if(voltage <= table.front().voltage)
                return table.front().value;
            if(voltage >= table.back().voltage)
                return table.back().value;
        }

        // Binary search for the correct interval
        auto upper = std::lower_bound(
            table.begin(), table.end(), voltage,
            [](const CalibrationData& data, float v) {
                return data.voltage < v;
            });

        auto lower = upper - 1;

        float x0 = lower->voltage;
        float x1 = upper->voltage;
        float y0 = lower->value;
        float y1 = upper->value;

        float ratio = 0.0f;
        if(x1 != x0)
            ratio = (voltage - x0) / (x1 - x0);

        return y0 + ratio * (y1 - y0);
    }

    const std::shared_ptr<std::vector<CalibrationData>> GetCalibrationTable() const override {
        return calibration_table_;
    }

    const InterpolationMethod GetInterpolationMethod() const override {
        return INTERPOLATION_METHOD;
    }
};

} // namespace eerie_leap::utilities::voltage_interpolator

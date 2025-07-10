#pragma once

#include <memory>
#include <vector>
#include <algorithm>

#include "i_voltage_interpolator.h"
#include "calibration_data.h"
#include "interpolation_method.h"

namespace eerie_leap::utilities::voltage_interpolator {

class CubicSplineVoltageInterpolator : public IVoltageInterpolator {
private:
    static const InterpolationMethod INTERPOLATION_METHOD = InterpolationMethod::CUBIC_SPLINE;
    std::shared_ptr<std::vector<CalibrationData>> calibration_table_;
    std::vector<float> h_, alpha_, l_, mu_, z_;
    std::vector<float> a_, b_, c_, d_;

public:
    explicit CubicSplineVoltageInterpolator(const std::shared_ptr<std::vector<CalibrationData>>& calibration_table)
        : calibration_table_(std::move(calibration_table)) {

        if(!calibration_table_ || calibration_table_->size() < 2)
            throw std::invalid_argument("Calibration data is missing or invalid.");

        const auto& table = *calibration_table_;

        const size_t n = table.size();
        h_.resize(n - 1);
        alpha_.resize(n - 1);
        l_.resize(n);
        mu_.resize(n);
        z_.resize(n);
        a_.resize(n);
        b_.resize(n - 1);
        c_.resize(n);
        d_.resize(n - 1);

        for(size_t i = 0; i < n; ++i)
            a_[i] = table[i].value;

        for(size_t i = 0; i < n - 1; ++i)
            h_[i] = table[i + 1].voltage - table[i].voltage;

        for(size_t i = 1; i < n - 1; ++i)
            alpha_[i] = (3.0f / h_[i]) * (a_[i + 1] - a_[i]) - (3.0f / h_[i - 1]) * (a_[i] - a_[i - 1]);

        l_[0] = 1.0f;
        mu_[0] = 0.0f;
        z_[0] = 0.0f;

        for(size_t i = 1; i < n - 1; ++i) {
            l_[i] = 2.0f * (table[i + 1].voltage - table[i - 1].voltage) - h_[i - 1] * mu_[i - 1];
            mu_[i] = h_[i] / l_[i];
            z_[i] = (alpha_[i] - h_[i - 1] * z_[i - 1]) / l_[i];
        }

        l_[n - 1] = 1.0f;
        z_[n - 1] = 0.0f;
        c_[n - 1] = 0.0f;

        for(int j = n - 2; j >= 0; --j) {
            c_[j] = z_[j] - mu_[j] * c_[j + 1];
            b_[j] = (a_[j + 1] - a_[j]) / h_[j] - h_[j] * (c_[j + 1] + 2.0f * c_[j]) / 3.0f;
            d_[j] = (c_[j + 1] - c_[j]) / (3.0f * h_[j]);
        }
    }

    float Interpolate(float voltage, bool clamp_to_ends = false) const override {
        const auto& table = *calibration_table_;

        if(clamp_to_ends) {
            if(voltage <= table.front().voltage)
                return table.front().value;
            if(voltage >= table.back().voltage)
                return table.back().value;
        }

        auto it = std::upper_bound(table.begin(), table.end(), voltage,
            [](float val, const CalibrationData& d) { return val < d.voltage; });

        size_t i = std::distance(table.begin(), it) - 1;
        float dx = voltage - table[i].voltage;

        return a_[i] + b_[i] * dx + c_[i] * dx * dx + d_[i] * dx * dx * dx;
    }

    const std::shared_ptr<std::vector<CalibrationData>> GetCalibrationTable() const override {
        return calibration_table_;
    }

    const InterpolationMethod GetInterpolationMethod() const override {
        return INTERPOLATION_METHOD;
    }
};

} // namespace eerie_leap::utilities::voltage_interpolator

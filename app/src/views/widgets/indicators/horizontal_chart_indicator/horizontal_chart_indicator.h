#pragma once

#include <memory>

#include <lvgl.h>

#include "views/widgets/indicators/i_indicator.h"

namespace eerie_leap::views::widgets::indicators {

enum class HorizontalChartIndicatorType : uint8_t {
    BAR,
    LINE,
};

class HorizontalChartIndicator : public IIndicator {
private:
    IndicatorState state_;
    HorizontalChartIndicatorType type_;
    int32_t point_count_;

    static lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end, int32_t point_count, HorizontalChartIndicatorType type);

public:
    HorizontalChartIndicator(float range_start, float range_end, int32_t point_count, HorizontalChartIndicatorType type);

    int Render() override;
    void Update(float value) override;

    const IndicatorState* GetState() const override { return &state_; }
};

} // namespace eerie_leap::views::widgets::indicators

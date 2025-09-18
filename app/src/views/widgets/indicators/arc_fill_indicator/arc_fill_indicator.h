#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/i_indicator.h"

namespace eerie_leap::views::widgets::indicators {

class ArcFillIndicator : public IIndicator {
private:
    IndicatorState state_;

    static lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

public:
    ArcFillIndicator(float range_start, float range_end);

    int Render() override;
    void Update(float value) override;

    const IndicatorState* GetState() const override { return &state_; }
};

} // namespace eerie_leap::views::widgets::indicators

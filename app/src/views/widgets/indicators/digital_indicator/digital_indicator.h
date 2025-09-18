#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/i_indicator.h"

namespace eerie_leap::views::widgets::indicators {

class DigitalIndicator : public IIndicator {
private:
    IndicatorState state_;

    static lv_obj_t* Create(lv_obj_t* parent);

public:
    DigitalIndicator(float range_start, float range_end);

    int Render() override;
    void Update(float value) override;

    const IndicatorState* GetState() const override { return &state_; }
};

} // namespace eerie_leap::views::widgets::indicators

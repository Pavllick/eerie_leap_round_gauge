#pragma once

#include <list>
#include <memory>

#include <lvgl.h>

#include "views/widgets/indicators/i_indicator.h"

namespace eerie_leap::views::screens {

using namespace eerie_leap::views::widgets::indicators;

struct RadialDigitalGaugeState {
    int32_t range;
    int32_t value;
    std::list<std::shared_ptr<IIndicator>> indicators;
};

class RadialDigitalGauge {
private:
    lv_anim_t value_change_animation_;

    RadialDigitalGaugeState state_;

    lv_anim_t CreateValueChangeAnimation();
    static void UpdateIndicator(void* obj, int32_t value);

    void ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value);

public:
    RadialDigitalGauge(float range_start, float range_end);

    int Render();
    void Update(float value);
};

} // namespace eerie_leap::views::screens

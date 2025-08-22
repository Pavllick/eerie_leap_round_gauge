#pragma once

#include <lvgl.h>

namespace eerie_leap::views::widgets::indicators {

struct IndicatorState {
    lv_obj_t* lv_obj;
    float range_start;
    float range_end;
    float value;
};

class IIndicator {
public:
    virtual int Render() = 0;
    virtual void Update(float value) = 0;
    virtual const IndicatorState* GetState() const = 0;
};

} // namespace eerie_leap::views::widgets::indicators

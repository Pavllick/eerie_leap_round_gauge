#pragma once

#include <memory>

#include <lvgl.h>

#include "views/widgets/utilitites/frame.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets::utilitites;

struct IndicatorState {
    std::shared_ptr<Frame> container;
    lv_obj_t* lv_obj;
    bool is_smooth;
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

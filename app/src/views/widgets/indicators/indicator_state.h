#pragma once

#include "views/widgets/widget_state.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets;

struct IndicatorState : public WidgetState {
    float range_start;
    float range_end;
    float value;
};

} // namespace eerie_leap::views::widgets::indicators

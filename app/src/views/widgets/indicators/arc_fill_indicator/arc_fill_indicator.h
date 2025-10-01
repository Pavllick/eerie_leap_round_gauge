#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

class ArcFillIndicator : public IndicatorBase {
private:
    void UpdateIndicator(int32_t value) override;

    static lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

    int DoRender() override;
    int ApplyTheme() override;

public:
    explicit ArcFillIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorArcFill; }
};

} // namespace eerie_leap::views::widgets::indicators

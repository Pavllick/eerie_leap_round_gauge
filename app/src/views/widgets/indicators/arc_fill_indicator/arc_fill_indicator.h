#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

class ArcFillIndicator : public IndicatorBase {
private:
    int start_angle_;
    int end_angle_;

    lv_obj_t* lv_arc_;

    void UpdateIndicator(float value) override;

    lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

    int DoRender() override;
    int ApplyTheme() override;

    void Configure(const WidgetConfiguration& config) override;

public:
    explicit ArcFillIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorArcFill; }
};

} // namespace eerie_leap::views::widgets::indicators

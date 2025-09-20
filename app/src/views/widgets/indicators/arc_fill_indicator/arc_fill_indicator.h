#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

class ArcFillIndicator : public IndicatorBase {
private:
    void UpdateIndicator(int32_t value) override;

    static lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

public:
    explicit ArcFillIndicator(uint32_t id);
    WidgetType GetType() const override { return WidgetType::IndicatorArcFill; }

    int Render() override;
    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::indicators

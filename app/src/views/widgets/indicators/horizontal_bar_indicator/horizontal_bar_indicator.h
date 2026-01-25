#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

class HorizontalBarIndicator : public IndicatorBase {
private:
    lv_obj_t* lv_bar_;

    void UpdateIndicator(float value) override;

    lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

    int DoRender() override;
    int ApplyTheme() override;

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;

public:
    explicit HorizontalBarIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorHorizontalBar; }
};

} // namespace eerie_leap::views::widgets::indicators

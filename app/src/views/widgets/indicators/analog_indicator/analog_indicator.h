#pragma once

#include <lvgl.h>

#include "views/widgets/basic/icon_widget/icon_widget.h"
#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets::basic;

class AnalogIndicator : public IndicatorBase {
private:
    int start_angle_;
    int end_angle_;

    static constexpr int DEFAULT_START_ANGLE = 45;
    static constexpr int DEFAULT_END_ANGLE = 315;

    lv_obj_t* lv_arc_;
    std::unique_ptr<IconWidget> needle_icon_;

    void UpdateIndicator(float value) override;

    lv_obj_t* Create(int32_t range_start, int32_t range_end);

    int DoRender() override;
    int ApplyTheme() override;

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;

    uint32_t GetAngleForValue(float value);

public:
    explicit AnalogIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorAnalog; }
};

} // namespace eerie_leap::views::widgets::indicators

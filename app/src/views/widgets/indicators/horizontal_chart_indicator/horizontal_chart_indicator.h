#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

enum class HorizontalChartIndicatorType : uint8_t {
    None = 0,
    Bar,
    Line,
};

class HorizontalChartIndicator : public IndicatorBase {
private:
    int32_t point_count_;
    HorizontalChartIndicatorType chart_type_;
    lv_obj_t* lv_chart_;

    void UpdateIndicator(float value) override;
    lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end, int32_t point_count, HorizontalChartIndicatorType type);

    int DoRender() override;
    int ApplyTheme() override;

public:
    explicit HorizontalChartIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorHorizontalChart; }

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
};

} // namespace eerie_leap::views::widgets::indicators

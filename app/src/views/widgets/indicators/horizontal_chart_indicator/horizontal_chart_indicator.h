#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

enum class HorizontalChartIndicatorType : uint8_t {
    BAR,
    LINE,
};

class HorizontalChartIndicator : public IndicatorBase {
private:
    int32_t point_count_;
    HorizontalChartIndicatorType chart_type_;

    void UpdateIndicator(int32_t value) override;
    static lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end, int32_t point_count, HorizontalChartIndicatorType type);

public:
    explicit HorizontalChartIndicator(uint32_t id);
    WidgetType GetType() const override { return WidgetType::IndicatorHorizontalChart; }

    int Render() override;
    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::indicators

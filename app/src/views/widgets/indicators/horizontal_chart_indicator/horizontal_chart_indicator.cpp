#include <stdexcept>

#include "views/widgets/utilitites/frame.h"

#include "horizontal_chart_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets::utilitites;

HorizontalChartIndicator::HorizontalChartIndicator(float range_start, float range_end, int32_t point_count, HorizontalChartIndicatorType type) : state_(), type_(type), point_count_(point_count) {
    state_.is_smooth = false;
    state_.range_start = range_start;
    state_.range_end = range_end;
    state_.value = range_start;

    auto container = std::make_shared<Frame>();
    container->Build();
    state_.container = std::move(container);
}

int HorizontalChartIndicator::Render() {
    state_.lv_obj = Create(state_.container->GetObject(), static_cast<int32_t>(state_.range_start), static_cast<int32_t>(state_.range_end), point_count_, type_);

    return 0;
}

lv_obj_t* HorizontalChartIndicator::Create(lv_obj_t* parent, int32_t range_start, int32_t range_end, int32_t point_count, HorizontalChartIndicatorType type) {
    lv_obj_t * chart = lv_chart_create(parent);
    lv_obj_remove_style_all(chart);

    lv_chart_type_t chart_type = LV_CHART_TYPE_NONE;
    switch(type) {
        case HorizontalChartIndicatorType::BAR:
            chart_type = LV_CHART_TYPE_BAR;
            lv_obj_set_style_pad_column(chart, 2, LV_PART_MAIN);
            break;
        case HorizontalChartIndicatorType::LINE:
            chart_type = LV_CHART_TYPE_LINE;
            lv_obj_set_style_line_width(chart, 6, LV_PART_ITEMS);
            break;
    }

    if(chart_type == LV_CHART_TYPE_NONE)
        throw std::runtime_error("Invalid chart type");

    lv_chart_set_type(chart, chart_type);

    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
    lv_obj_set_width(chart, lv_pct(100));
    lv_obj_set_height(chart, lv_pct(100));

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, range_start, range_end);

    lv_chart_set_point_count(chart, point_count);
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_color_hex(0x000000), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_series_color(chart, ser, lv_color_hex(0x3D3D3D));

    lv_obj_set_style_line_opa(chart, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_DEFAULT);

    return chart;
}

lv_obj_t* Create2(lv_obj_t* parent, int32_t range_start, int32_t range_end, int32_t point_count, HorizontalChartIndicatorType type) {
    lv_obj_t * chart = lv_chart_create(parent);
    lv_obj_remove_style_all(chart);

    lv_chart_type_t chart_type = LV_CHART_TYPE_NONE;
    switch(type) {
        case HorizontalChartIndicatorType::BAR:
            chart_type = LV_CHART_TYPE_BAR;
            break;
        case HorizontalChartIndicatorType::LINE:
            chart_type = LV_CHART_TYPE_LINE;
            break;
    }

    if(chart_type == LV_CHART_TYPE_NONE)
        throw std::runtime_error("Invalid chart type");

    lv_chart_set_type(chart, chart_type);

    // Essential styles for both chart types
    lv_obj_set_style_bg_opa(chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(chart, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Line-specific styles (these are crucial for line charts)
    if(chart_type == LV_CHART_TYPE_LINE) {
        lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
        lv_obj_set_style_line_opa(chart, LV_OPA_COVER, LV_PART_ITEMS);
        // lv_obj_set_style_size(chart, 4, LV_PART_INDICATOR); // Point size
        lv_obj_set_style_bg_opa(chart, LV_OPA_COVER, LV_PART_INDICATOR); // Point visibility
        lv_obj_set_style_radius(chart, LV_RADIUS_CIRCLE, LV_PART_INDICATOR); // Round points
    }

    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

    // For bar charts, hide the indicators (points)
    // if(chart_type == LV_CHART_TYPE_BAR) {
    //     lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
    // }

    lv_obj_set_width(chart, lv_pct(100));
    lv_obj_set_height(chart, lv_pct(100));

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, range_start, range_end);

    lv_chart_set_point_count(chart, point_count);
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_color_hex(0x000000), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_series_color(chart, ser, lv_color_hex(0x3D3D3D));

    // Line color styling
    if(chart_type == LV_CHART_TYPE_LINE) {
        lv_obj_set_style_line_color(chart, lv_color_hex(0x4040FF), LV_PART_ITEMS);
    } else {
        lv_obj_set_style_line_opa(chart, 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
        lv_obj_set_style_line_color(chart, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    return chart;
}

void HorizontalChartIndicator::Update(float value) {
    lv_obj_t * chart = state_.lv_obj;
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, (int32_t)value);

    state_.value = value;
}

} // namespace eerie_leap::views::widgets::indicators

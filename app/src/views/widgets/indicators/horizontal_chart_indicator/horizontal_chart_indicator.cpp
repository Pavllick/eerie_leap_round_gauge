#include <stdexcept>
#include <string>

#include "utilities/type/config_value.h"
#include "domain/ui_domain/models/widget_property.h"

#include "horizontal_chart_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

HorizontalChartIndicator::HorizontalChartIndicator(uint32_t id, std::shared_ptr<Frame> parent)
    : IndicatorBase(id, parent) { }

int HorizontalChartIndicator::DoRender() {
    auto lv_obj = Create(container_->GetObject(), static_cast<int32_t>(range_start_), static_cast<int32_t>(range_end_), point_count_, chart_type_);
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int HorizontalChartIndicator::ApplyTheme() {
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

void HorizontalChartIndicator::UpdateIndicator(int32_t value) {
    lv_obj_t * chart = container_->GetChild()->GetObject();
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, value);
    value_ = static_cast<float>(value);
}

void HorizontalChartIndicator::Configure(const WidgetConfiguration& config) {
    IndicatorBase::Configure(config);

    point_count_ = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT),
        30);

    chart_type_ = static_cast<HorizontalChartIndicatorType>(GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::CHART_TYPE),
        static_cast<std::uint16_t>(HorizontalChartIndicatorType::BAR)));
}

} // namespace eerie_leap::views::widgets::indicators

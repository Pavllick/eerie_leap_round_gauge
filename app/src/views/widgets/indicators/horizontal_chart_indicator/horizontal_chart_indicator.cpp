#include <stdexcept>
#include <string>

#include "utilities/type/config_value.h"
#include "views/widgets/configuration/widget_property.h"
#include "views/widgets/utilitites/frame.h"

#include "horizontal_chart_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::views::widgets::configuration;
using namespace eerie_leap::views::widgets::utilitites;

HorizontalChartIndicator::HorizontalChartIndicator(uint32_t id) : id_(id), state_() {
    auto container = std::make_shared<Frame>();
    container->Build();
    state_.container = std::move(container);

    value_change_animation_ = CreateValueChangeAnimation();
}

int HorizontalChartIndicator::Render() {
    state_.lv_obj = Create(state_.container->GetObject(), static_cast<int32_t>(state_.range_start), static_cast<int32_t>(state_.range_end), point_count_, chart_type_);

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

bool HorizontalChartIndicator::IsAnimationEnabled() const {
    return animation_enabled_;
}

void HorizontalChartIndicator::UpdateIndicator(void* obj, int32_t value) {
    auto* state = (IndicatorState*)obj;
    lv_obj_t * chart = state->lv_obj;
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, value);
    state->value = static_cast<float>(value);
}

lv_anim_t HorizontalChartIndicator::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, &state_);
    lv_anim_set_exec_cb(&anim, UpdateIndicator);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

void HorizontalChartIndicator::ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_start(&anim);
}

void HorizontalChartIndicator::Update(float value) {
    if(animation_enabled_)
        ValueChangeAnimation(value_change_animation_, state_.range_end - state_.range_start, state_.value, static_cast<int32_t>(value));
    else
        UpdateIndicator(&state_, static_cast<int>(value));
}

void HorizontalChartIndicator::Configure(const WidgetConfiguration& config) {
    configuration_ = config;

    animation_enabled_ = config.is_animation_enabled;

    auto sensor_id_str = GetConfigValue<std::string>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID),
        "");
    if(!sensor_id_str.empty())
        sensor_id_ = std::stoul(sensor_id_str);

    state_.range_start = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE),
        0);
    state_.range_end = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE),
        100);
    state_.value = state_.range_start;

    point_count_ = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::CHART_POINT_COUNT),
        30);

    // TODO: move to relevant config sections
    state_.container->SetHeight(140, true).AlignBottom();
}

const IndicatorState* HorizontalChartIndicator::GetState() const {
    return &state_;
}

uint32_t HorizontalChartIndicator::GetId() const {
    return id_;
}

std::optional<uint32_t> HorizontalChartIndicator::GetSensorId() const {
    return sensor_id_;
}

WidgetConfiguration HorizontalChartIndicator::GetConfiguration() const {
    return configuration_;
}

WidgetPosition HorizontalChartIndicator::GetPosition() const {
    return position_;
}

void HorizontalChartIndicator::SetPosition(const WidgetPosition& pos) {
    position_ = pos;
}

WidgetSize HorizontalChartIndicator::GetSize() const {
    return size_;
}

void HorizontalChartIndicator::SetSize(const WidgetSize& size) {
    size_ = size;
}

} // namespace eerie_leap::views::widgets::indicators

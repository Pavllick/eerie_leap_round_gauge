#include "views/widgets/configuration/widget_property.h"

#include "indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets::configuration;

IndicatorBase::IndicatorBase(uint32_t id) : WidgetBase(id) {
    value_change_animation_ = CreateValueChangeAnimation();
}

void IndicatorBase::UpdateIndicatorCallback(void* obj, int32_t value) {
    auto* indicator = static_cast<IndicatorBase*>(obj);
    indicator->UpdateIndicator(value);
}

lv_anim_t IndicatorBase::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, this);
    lv_anim_set_exec_cb(&anim, UpdateIndicatorCallback);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

void IndicatorBase::ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_start(&anim);
}

void IndicatorBase::Update(float value) {
    if(IsAnimated()) {
        ValueChangeAnimation(
            value_change_animation_,
            static_cast<int32_t>(range_end_ - range_start_),
            static_cast<int32_t>(value_),
            static_cast<int32_t>(value));
    } else {
        UpdateIndicator(static_cast<int32_t>(value));
    }
}

std::optional<uint32_t> IndicatorBase::GetSensorId() const {
    return sensor_id_;
}

void IndicatorBase::Configure(const WidgetConfiguration& config) {
    WidgetBase::Configure(config);

    auto sensor_id_str = GetConfigValue<std::string>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID),
        "");
    if(!sensor_id_str.empty())
        sensor_id_ = std::stoul(sensor_id_str);

    range_start_ = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE),
        0);
    range_end_ = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE),
        100);
    value_ = range_start_;
}

} // namespace eerie_leap::views::widgets::indicators

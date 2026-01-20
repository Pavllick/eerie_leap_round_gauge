#include <variant>

#include "utilities/string/string_helpers.h"
#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "domain/ui_domain/models/widget_property.h"
#include "views/widgets/event_bus_filters/sensor_filter.h"

#include "indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::string;
using namespace eerie_leap::domain::ui_domain::event_bus;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets::event_bus_filters;

IndicatorBase::IndicatorBase(uint32_t id, std::shared_ptr<Frame> parent)
    : WidgetBase(id, parent) , value_filter_(0) {

    value_change_animation_ = CreateValueChangeAnimation();
}

void IndicatorBase::UpdateIndicatorCallback(void* obj, int32_t value) {
    float value_float = static_cast<float>(value) / (10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS);

    auto* indicator = static_cast<IndicatorBase*>(obj);

    indicator->UpdateIndicator(value_float);
    indicator->value_ = value_float;
}

lv_anim_t IndicatorBase::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, this);
    lv_anim_set_exec_cb(&anim, UpdateIndicatorCallback);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

// void IndicatorBase::ValueChangeAnimation(lv_anim_t& anim, float range, float start_value, float end_value) {
//     uint32_t duration = 4000;
//     uint32_t unit_duration = duration / range;
//     uint32_t change_duration = unit_duration * abs(end_value - start_value);

//     lv_anim_set_duration(&anim, change_duration);
//     lv_anim_set_values(&anim,
//         static_cast<int32_t>(start_value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS),
//         static_cast<int32_t>(end_value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS));
//     lv_anim_start(&anim);
// }

void IndicatorBase::Update(float value) {
    if(!IsReady())
        return;

    int32_t scaled_value = static_cast<int32_t>(
        value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS);

    if(IsSmoothed()) {
        int32_t filtered = value_filter_.Filter(scaled_value, smoothing_factor_);
        UpdateIndicatorCallback(this, filtered);
    } else {
        UpdateIndicatorCallback(this, scaled_value);
    }
}

// void IndicatorBase::Update(float value) {
//     if(!IsReady())
//         return;

//     if(IsSmoothed()) {
//         ValueChangeAnimation(
//             value_change_animation_,
//             range_end_ - range_start_,
//             value_,
//             value);
//     } else {
//         UpdateIndicatorCallback(this, static_cast<int32_t>(value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS));
//     }
// }

std::optional<size_t> IndicatorBase::GetSensorIdHash() const {
    return sensor_id_hash_;
}

void IndicatorBase::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    WidgetBase::Configure(configuration);

    auto sensor_id = GetConfigValue<std::pmr::string>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID),
        "");
    if(!sensor_id.empty()) {
        sensor_id_hash_ = StringHelpers::GetHash(sensor_id);

        auto result = UiEventBus::GetInstance().Subscribe(
            UiEventType::SensorDataUpdated,
            SensorFilter { sensor_id_hash_.value() },
            [this](const UiEvent& event) {
                if (auto it = event.payload.find(UiPayloadType::Value); it != event.payload.end()) {
                    if (auto* value = std::get_if<float>(&it->second)) {
                        this->Update(*value);
                    }
                }
            }
        );

        if(result)
            subscriptions_.push_back(std::move(*result));
    }

    range_start_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE),
        0);
    range_end_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE),
        100);
    value_ = range_start_;
}

} // namespace eerie_leap::views::widgets::indicators

#include <cmath>
#include <optional>
#include <variant>

#include "utilities/string/string_helpers.h"
#include "domain/sensor_domain/event_bus/sensor_events_channel.h"
#include "domain/ui_domain/event_bus/ui_signal_channel.h"
#include "domain/ui_domain/models/indicator_value_source.h"
#include "domain/ui_domain/models/widget_property.h"
#include "views/widgets/event_bus_filters/sensor_filter.h"

#include "indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::string;
using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::sensor_domain::event_bus;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets::event_bus_filters;

using eerie_leap::domain::ui_domain::event_bus::UiSignalType;
using eerie_leap::domain::ui_domain::event_bus::UiSignalChannel;
using eerie_leap::domain::ui_domain::event_bus::UiSignalPayloadType;

IndicatorBase::IndicatorBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : WidgetBase(id, std::move(parent), std::move(context)) , value_filter_(0) {

    value_change_animation_ = CreateValueChangeAnimation();
}

IndicatorBase::~IndicatorBase() {
    DetachDispatch();

    // The animation targets `this`, so it must not outlive the indicator.
    lv_anim_delete(this, UpdateIndicatorCallback);
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

void IndicatorBase::ValueChangeAnimation(lv_anim_t& anim, float range, float start_value, float end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim,
        static_cast<int32_t>(start_value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS),
        static_cast<int32_t>(end_value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS));
    lv_anim_start(&anim);
}

// void IndicatorBase::Update(float value) {
//     if(!IsReady())
//         return;

//     int32_t scaled_value = static_cast<int32_t>(
//         value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS);

//     if(IsSmoothed()) {
//         int32_t filtered = value_filter_.Filter(scaled_value, smoothing_factor_);
//         UpdateIndicatorCallback(this, filtered);
//     } else {
//         UpdateIndicatorCallback(this, scaled_value);
//     }
// }

// NOTE: Actual LVGL animation renders frames smoother
// compared to Exponential Moving Average Filter which
// can be jumpy and result in noticeable tearing effect
void IndicatorBase::Update(float value) {
    if(!IsReady())
        return;

    if(value < range_start_)
        value = range_start_;
    else if(value > range_end_)
        value = range_end_;

    if(IsSmoothed()) {
        if(std::abs(value - value_) < min_animation_delta_ratio_ * (range_end_ - range_start_))
            return;

        ValueChangeAnimation(
            value_change_animation_,
            range_end_ - range_start_,
            value_,
            value);
    } else {
        UpdateIndicatorCallback(this, static_cast<int32_t>(value * 10 * CONFIG_EERIE_LEAP_FLOAT_SIGNIFICANT_DIGITS));
    }
}

void IndicatorBase::OnDeactivated() {
    WidgetBase::OnDeactivated();

    // A running animation keeps invalidating the widget after its group is hidden.
    lv_anim_delete(this, UpdateIndicatorCallback);
}

std::optional<uint32_t> IndicatorBase::GetSensorIdHash() const {
    return sensor_id_hash_;
}

void IndicatorBase::SubscribeToSensor() {
    auto sensor_id = properties_->GetAs<std::pmr::string>(WidgetPropertyType::SENSOR_ID, "");
    if(sensor_id.empty())
        return;

    sensor_id_hash_ = StringHelpers::GetHash(sensor_id);

    SubscribeWhileActive(
        SensorEventsChannel::GetInstance(),
        SensorEventType::DataUpdated,
        SensorFilter { sensor_id_hash_.value() },
        [this](const SensorEventsChannel::EventMessage& event) {
            if (auto it = event.payload.find(SensorPayloadType::Value); it != event.payload.end()) {
                if (auto* value = std::get_if<float>(&it->second)) {
                    this->Update(*value);
                }
            }
        });
}

void IndicatorBase::SubscribeToUiSignal() {
    auto signal = static_cast<UiSignalType>(
        properties_->GetAs<int>(WidgetPropertyType::UI_SIGNAL_TYPE, 0));
    if(signal == UiSignalType::None)
        return;

    SubscribeWhileActive(
        UiSignalChannel::GetInstance(),
        signal,
        [this](const UiSignalChannel::EventMessage& event) {
            if(auto it = event.payload.find(UiSignalPayloadType::Value); it != event.payload.end()) {
                if(auto value = ValueToFloat(it->second)) {
                    this->Update(*value);
                }
            }
        });
}

void IndicatorBase::RegisterProperties(WidgetPropertyStore& store) {
    WidgetBase::RegisterProperties(store);

    store.Register(WidgetPropertyType::MIN_VALUE, ConfigValue { 0 }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::MAX_VALUE, ConfigValue { 100 }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::VALUE, ConfigValue { 0.0 }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::SENSOR_ID, ConfigValue { std::pmr::string { } }, PropertyChangeEffect::None);
    store.Register(WidgetPropertyType::UI_SIGNAL_TYPE, ConfigValue { 0 }, PropertyChangeEffect::None);
    store.Register(
        WidgetPropertyType::VALUE_SOURCE,
        ConfigValue { static_cast<int>(IndicatorValueSource::Sensor) },
        PropertyChangeEffect::None);
}

void IndicatorBase::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    switch(type) {
        case WidgetPropertyType::MIN_VALUE:
            range_start_ = ConfigValueAs<double>(value, 0);
            break;

        case WidgetPropertyType::MAX_VALUE:
            range_end_ = ConfigValueAs<double>(value, 100);
            break;

        case WidgetPropertyType::VALUE:
            Update(static_cast<float>(ConfigValueAs<double>(value, 0)));
            break;

        default:
            WidgetBase::OnPropertyChanged(type, value);
            break;
    }
}

void IndicatorBase::OnConfigured() {
    WidgetBase::OnConfigured();

    auto value_source = static_cast<IndicatorValueSource>(properties_->GetAs<int>(
        WidgetPropertyType::VALUE_SOURCE,
        static_cast<int>(IndicatorValueSource::Sensor)));

    if(value_source == IndicatorValueSource::UiSignal)
        SubscribeToUiSignal();
    else
        SubscribeToSensor();
}

// A signal carries whatever numeric type its source domain published.
std::optional<float> IndicatorBase::ValueToFloat(const EventData& value) {
    if(const auto* as_float = std::get_if<float>(&value))
        return *as_float;
    if(const auto* as_int = std::get_if<int>(&value))
        return static_cast<float>(*as_int);
    if(const auto* as_uint = std::get_if<uint32_t>(&value))
        return static_cast<float>(*as_uint);

    return std::nullopt;
}

} // namespace eerie_leap::views::widgets::indicators

#pragma once

#include <optional>

#include "utilities/math/ema_filter.hpp"
#include "subsys/event_bus/event.h"
#include "views/widgets/indicators/i_indicator.h"
#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::indicators {

using eerie_leap::subsys::event_bus::EventData;
using EmaFilterInt32 = eerie_leap::utilities::math::EmaFilter<int32_t>;

class IndicatorBase : public WidgetBase, public IIndicator {
private:
    static void UpdateIndicatorCallback(void* obj, int32_t value);

    EmaFilterInt32 value_filter_;
    // uint8_t smoothing_factor_ = 4;

    // Sensor samples can arrive faster than the value-change animation
    // can settle (ValueChangeAnimation() below), so without a dead-zone
    // a near-flat/noisy signal keeps restarting the animation before it
    // ever finishes - the widget stays "in motion" and gets invalidated/
    // redrawn on every render tick indefinitely, even with no visually
    // meaningful change. Skip re-triggering for tiny deltas.
    static constexpr float min_animation_delta_ratio_ = 0.001F; // 0.1% of range

    static std::optional<float> ValueToFloat(const EventData& value);

protected:
    std::optional<uint32_t> sensor_id_hash_;
    lv_anim_t value_change_animation_;
    float range_start_ = 0;
    float range_end_ = 0;
    float value_ = 0;

    virtual void UpdateIndicator(float value) = 0;
    lv_anim_t CreateValueChangeAnimation();
    void ValueChangeAnimation(lv_anim_t& anim, float range, float start_value, float end_value);

    void SubscribeToSensor();
    void SubscribeToUiSignal();

public:
    IndicatorBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context);
    ~IndicatorBase() override;

    void Update(float value) override;

    void OnDeactivated() override;

    std::optional<uint32_t> GetSensorIdHash() const override;
protected:
    void RegisterProperties(WidgetPropertyStore& store) override;
    void OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) override;
    void OnConfigured() override;
};

} // namespace eerie_leap::views::widgets::indicators

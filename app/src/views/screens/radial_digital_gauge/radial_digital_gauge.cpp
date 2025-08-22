#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "utilities/memory/heap_allocator.h"
#include "views/widgets/indicators/arc_fill_indicator/arc_fill_indicator.h"
#include "views/widgets/indicators/digital_indicator/digital_indicator.h"
#include "radial_digital_gauge.h"

namespace eerie_leap::views::screens {

using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(radial_digital_gauge_logger);

RadialDigitalGauge::RadialDigitalGauge(float range_start, float range_end) : state_() {
    state_.range = static_cast<int32_t>(range_end - range_start);
    state_.value = static_cast<int32_t>(range_start);

    auto arc_fill_indicator = ArcFillIndicator(range_start, range_end);
    auto digital_indicator = DigitalIndicator(range_start, range_end);
    state_.indicators.push_back(make_shared_ext<ArcFillIndicator>(arc_fill_indicator));
    state_.indicators.push_back(make_shared_ext<DigitalIndicator>(digital_indicator));
    value_change_animation_ = CreateValueChangeAnimation();
}

int RadialDigitalGauge::Render() {
    for(auto& indicator : state_.indicators)
        indicator->Render();

    return 0;
}

void RadialDigitalGauge::UpdateIndicator(void* obj, int32_t value) {
    auto* state = (RadialDigitalGaugeState*)obj;
    for(auto& indicator : state->indicators)
        indicator->Update(static_cast<float>(value));

    state->value = value;
}

lv_anim_t RadialDigitalGauge::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, &state_);
    lv_anim_set_exec_cb(&anim, UpdateIndicator);
    lv_anim_set_duration(&anim, 100);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

void RadialDigitalGauge::ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_start(&anim);
}

void RadialDigitalGauge::Update(float value) {
    ValueChangeAnimation(value_change_animation_, state_.range, state_.value, static_cast<int32_t>(value));

    // UpdateIndicator(&state_, static_cast<int>(value));
}

} // namespace eerie_leap::views::screens

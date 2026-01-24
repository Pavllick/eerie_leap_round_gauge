#pragma once

#include <optional>

#include "utilities/math/ema_filter.hpp"
#include "views/widgets/indicators/i_indicator.h"
#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::math;
using namespace eerie_leap::views::widgets;

class IndicatorBase : public WidgetBase, public IIndicator {
private:
    static void UpdateIndicatorCallback(void* obj, int32_t value);

    EmaFilter<int32_t> value_filter_;
    uint8_t smoothing_factor_ = 4;

protected:
    std::optional<size_t> sensor_id_hash_;
    lv_anim_t value_change_animation_;
    float range_start_ = 0;
    float range_end_ = 0;
    float value_ = 0;

    virtual void UpdateIndicator(float value) = 0;
    lv_anim_t CreateValueChangeAnimation();
    void ValueChangeAnimation(lv_anim_t& anim, float range, float start_value, float end_value);

public:
    IndicatorBase(uint32_t id, std::shared_ptr<Frame> parent);

    void Update(float value) override;

    std::optional<size_t> GetSensorIdHash() const override;
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
};

} // namespace eerie_leap::views::widgets::indicators

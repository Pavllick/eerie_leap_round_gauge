#pragma once

#include <optional>

#include "views/widgets/indicators/i_indicator.h"
#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets;

class IndicatorBase : public WidgetBase, public IIndicator {
private:
    static void UpdateIndicatorCallback(void* obj, int32_t value);

protected:
    std::optional<uint32_t> sensor_id_;
    lv_anim_t value_change_animation_;
    float range_start_;
    float range_end_;
    float value_;

    virtual void UpdateIndicator(int32_t value) = 0;
    lv_anim_t CreateValueChangeAnimation();
    void ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value);

public:
    IndicatorBase(uint32_t id, std::shared_ptr<Frame> parent);

    void Update(float value) override;

    std::optional<uint32_t> GetSensorId() const override;
    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::indicators

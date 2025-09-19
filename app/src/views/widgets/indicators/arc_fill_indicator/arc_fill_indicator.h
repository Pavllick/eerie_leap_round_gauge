#pragma once

#include <optional>

#include <lvgl.h>

#include "views/widgets/indicators/i_indicator.h"

namespace eerie_leap::views::widgets::indicators {

class ArcFillIndicator : public IIndicator {
private:
    uint32_t id_;
    IndicatorState state_;
    std::optional<uint32_t> sensor_id_;
    bool animation_enabled_;
    lv_anim_t value_change_animation_;

    WidgetConfiguration configuration_;
    WidgetPosition position_;
    WidgetSize size_;

    lv_anim_t CreateValueChangeAnimation();
    static void UpdateIndicator(void* obj, int32_t value);
    void ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value);

    static lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

public:
    explicit ArcFillIndicator(uint32_t id);
    WidgetType GetType() const override { return WidgetType::IndicatorArcFill; }

    int Render() override;
    void Update(float value) override;
    bool IsAnimationEnabled() const override;

    const IndicatorState* GetState() const override;
    uint32_t GetId() const override;
    std::optional<uint32_t> GetSensorId() const override;

    void Configure(const WidgetConfiguration& config) override;
    WidgetConfiguration GetConfiguration() const override;

    WidgetPosition GetPosition() const override;
    void SetPosition(const WidgetPosition& pos) override;
    WidgetSize GetSize() const override;
    void SetSize(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets::indicators

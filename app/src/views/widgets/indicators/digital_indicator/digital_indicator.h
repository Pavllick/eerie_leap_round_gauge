#pragma once

#include <lvgl.h>

#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

class DigitalIndicator : public IndicatorBase {
private:
    void UpdateIndicator(int32_t value) override;
    static lv_obj_t* Create(lv_obj_t* parent);

public:
    explicit DigitalIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorDigital; }

    int Render() override;
};

} // namespace eerie_leap::views::widgets::indicators

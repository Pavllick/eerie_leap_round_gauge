#pragma once

#include <lvgl.h>

#include "views/utilitites/enums.h"
#include "views/widgets/indicators/indicator_base.h"

namespace eerie_leap::views::widgets::indicators {

using namespace views::utilitites;

class BarIndicator : public IndicatorBase {
protected:
    lv_obj_t* lv_bar_;
    InidicatorDirection direction_;

    void UpdateIndicator(float value) override;

    virtual lv_obj_t* Create(lv_obj_t* parent, int32_t range_start, int32_t range_end);

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
    static void UpdateDirection(lv_obj_t* lv_bar, InidicatorDirection direction, int32_t range_start, int32_t range_end);

public:
    explicit BarIndicator(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::IndicatorBar; }
};

} // namespace eerie_leap::views::widgets::indicators

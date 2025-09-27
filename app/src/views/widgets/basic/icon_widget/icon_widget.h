#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::configuration;

class IconWidget : public WidgetBase {
private:
    lv_obj_t* Create(lv_obj_t* parent);

    // float start_angle_;
    // int radius_;
    // int center_x_;
    // int center_y_;

public:
    explicit IconWidget(uint32_t id);
    WidgetType GetType() const override { return WidgetType::BasicIcon; }

    int Render() override;
    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::basic

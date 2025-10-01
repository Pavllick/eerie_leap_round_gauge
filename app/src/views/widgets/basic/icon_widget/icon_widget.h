#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;

class IconWidget : public WidgetBase {
private:
    lv_obj_t* Create(lv_obj_t* parent);
    std::string label;

    // float start_angle_;
    // int radius_;
    // int center_x_;
    // int center_y_;

    int DoRender() override;
    int ApplyTheme() override;

public:
    explicit IconWidget(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::BasicIcon; }

    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::basic

#pragma once

#include <string>

#include <lvgl.h>

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;

class LabelIconWidget : public WidgetBase {
protected:
    lv_obj_t* Create(lv_obj_t* parent);
    int position_x_;
    int position_y_;
    std::string label;
    bool is_active_;

    lv_obj_t* lv_icon_;
    lv_obj_t* lv_label_;

    int DoRender() override;
    int ApplyTheme() override;

public:
    explicit LabelIconWidget(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::BasicLabelIcon; }

    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::basic

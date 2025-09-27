#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;

class ArcLabelWidget : public WidgetBase {
private:
    lv_obj_t* Create(lv_obj_t* parent);

    std::string text_;
    std::vector<lv_obj_t*> char_objects_;
    const lv_font_t* font_;

    float start_angle_;
    float end_angle_;
    int radius_;
    int center_x_;
    int center_y_;

    void CreateArcLabel(lv_obj_t* parent, const std::string& text, const lv_font_t* font,
        float start_angle, float end_angle_offset, int radius, int center_x, int center_y);

public:
    explicit ArcLabelWidget(uint32_t id);
    WidgetType GetType() const override { return WidgetType::BasicArcLabel; }

    int Render() override;
    void Configure(const WidgetConfiguration& config) override;
    void UpdateText(const std::string& value, float end_angle_offset = -1);
};

} // namespace eerie_leap::views::widgets::basic

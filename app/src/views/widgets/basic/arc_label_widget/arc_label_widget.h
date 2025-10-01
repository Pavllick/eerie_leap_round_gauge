#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;

class ArcLabelWidget : public WidgetBase {
private:
    void Create();

    std::string text_;
    std::vector<lv_obj_t*> char_objects_;
    const lv_font_t* font_;

    float start_angle_;
    float end_angle_;
    int radius_;
    int center_x_;
    int center_y_;

    int DoRender() override;
    int ApplyTheme() override;
    void CreateArcLabel(const std::string& text, const lv_font_t* font,
        float start_angle, float end_angle_offset, int radius, int center_x, int center_y);

public:
    explicit ArcLabelWidget(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::BasicArcLabel; }

    std::shared_ptr<Frame> UpdateText(const std::string& value, float end_angle_offset = -1);
};

} // namespace eerie_leap::views::widgets::basic

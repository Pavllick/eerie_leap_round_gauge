#include <string>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

#include "views/widgets/utilitites/positioning_helpers.h"

#include "arc_label_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets::utilitites;

ArcLabelWidget::ArcLabelWidget(uint32_t id) : WidgetBase(id), text_("") { }

int ArcLabelWidget::Render() {
    lv_obj_ = Create(container_->GetObject());

    return 0;
}

void ArcLabelWidget::CreateArcLabel(lv_obj_t* parent, const std::string& text, const lv_font_t* font,
    float start_angle, float end_angle_offset, int radius, int center_x, int center_y) {

    text_ = text;
    font_ = font;
    start_angle_ = start_angle;
    end_angle_ = start_angle_ + end_angle_offset;
    radius_ = radius;
    center_x_ = center_x;
    center_y_ = center_y;

    UpdateText(text_);
}

void ArcLabelWidget::UpdateText(const std::string& text, float end_angle_offset) {
    text_ = text;
    if(end_angle_offset != -1)
        end_angle_ = start_angle_ + end_angle_offset;

    auto parent = container_->GetObject();
    float angle_range = end_angle_ - start_angle_;
    char_objects_.clear();

    float angle_step = angle_range / (text.length() + 1);
    float angle = start_angle_ + angle_step;

    std::string char_text = text_;
    std::reverse(char_text.begin(), char_text.end());

    // // Reverse text for certain angle ranges
    // if ((90 < start_angle && start_angle < 180) ||
    //     (270 < start_angle && start_angle < 360)) {
    //     std::reverse(char_text.begin(), char_text.end());
    // }

    for (char c : char_text) {
        int t_angle = 2700 - (3600 - static_cast<int>(round(angle * 10)));

        if (180 < start_angle_ && 360 > end_angle_) {
            t_angle += 1800;
        }

        lv_obj_t* label = lv_label_create(parent);
        lv_obj_set_style_text_font(label, font_, LV_PART_MAIN);
        lv_label_set_text(label, std::string(1, c).c_str());
        lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), LV_PART_MAIN);
        lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(label, LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
        char_objects_.push_back(label);

        PositioningHelpers::PlaceObjectOnCircle(label, center_x_, center_y_, radius_, angle);
        angle += angle_step;
    }
}

// TODO: Make parameters configurable
lv_obj_t* ArcLabelWidget::Create(lv_obj_t* parent) {
    CreateArcLabel(
        parent,                 // Parent object
        "test",                 // Text to display
        &lv_font_montserrat_16, // Font to use
        70.0f,                 // Start angle (degrees)
        40.0f,                  // End angle offset (degrees)
        100,                    // Radius from center
        160,                    // Center X coordinate
        120                     // Center Y coordinate
    );

    return parent;
}

void ArcLabelWidget::Configure(const WidgetConfiguration& config) {
    WidgetBase::Configure(config);
}

} // namespace eerie_leap::views::widgets::basic

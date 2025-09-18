#include <string>

#include "views/widgets/utilitites/frame.h"

#include "digital_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets::utilitites;

DigitalIndicator::DigitalIndicator(float range_start, float range_end) : state_() {
    state_.is_smooth = true;
    state_.range_start = range_start;
    state_.range_end = range_end;
    state_.value = range_start;

    auto container = std::make_shared<Frame>();
    container->Build();
    state_.container = std::move(container);
}

int DigitalIndicator::Render() {
    state_.lv_obj = Create(state_.container->GetObject());

    return 0;
}

lv_obj_t* DigitalIndicator::Create(lv_obj_t* parent) {
    auto ui_label = lv_label_create(parent);

    lv_obj_set_width(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label, "0");
    lv_obj_set_style_text_font(ui_label, &lv_font_unscii_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_scale(ui_label, 1500, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_label, -65);
    lv_obj_set_y(ui_label, -55);

    return ui_label;
}

void DigitalIndicator::Update(float value) {
    auto int_value = static_cast<int32_t>(value);

    lv_label_set_text(state_.lv_obj, std::to_string(int_value).c_str());
    state_.value = value;
}

} // namespace eerie_leap::views::widgets::indicators

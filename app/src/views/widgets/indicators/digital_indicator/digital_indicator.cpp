#include <string>

#include "digital_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::views::widgets::configuration;
using namespace eerie_leap::views::widgets::utilitites;

DigitalIndicator::DigitalIndicator(uint32_t id) : IndicatorBase(id) { }

int DigitalIndicator::Render() {
    lv_obj_ = Create(container_->GetObject());

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

void DigitalIndicator::UpdateIndicator(int32_t value) {
    lv_label_set_text(lv_obj_, std::to_string(value).c_str());
    value_ = static_cast<float>(value);
}

void DigitalIndicator::Configure(const WidgetConfiguration& config) {
    IndicatorBase::Configure(config);
}

} // namespace eerie_leap::views::widgets::indicators

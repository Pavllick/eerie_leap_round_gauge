#include <zephyr/kernel.h>

#include "views/widgets/indicators/indicator_base.h"

#include "arc_fill_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

ArcFillIndicator::ArcFillIndicator(uint32_t id, std::shared_ptr<Frame> parent)
    : IndicatorBase(id, parent) { }

int ArcFillIndicator::DoRender() {
    lv_obj_ = Create(container_->GetObject(), static_cast<int32_t>(range_start_), static_cast<int32_t>(range_end_));

    return 0;
}

lv_obj_t* ArcFillIndicator::Create(lv_obj_t* parent, int32_t range_start, int32_t range_end) {
    lv_obj_t* ui_arc = lv_arc_create(parent);

    lv_arc_set_range(ui_arc, range_start, range_end);
    lv_obj_set_width(ui_arc, lv_pct(100));
    lv_obj_set_height(ui_arc, lv_pct(100));
    lv_obj_set_align(ui_arc, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_value(ui_arc, range_start);
    // lv_obj_set_style_arc_rounded(ui_arc, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_arc, lv_color_hex(0x1BBE5F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    return ui_arc;
}

void ArcFillIndicator::UpdateIndicator(int32_t value) {
    lv_arc_set_value(lv_obj_, value);
    value_ = static_cast<float>(value);
}

} // namespace eerie_leap::views::widgets::indicators

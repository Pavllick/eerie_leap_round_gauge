#include <zephyr/kernel.h>

#include "views/widgets/indicators/indicator_base.h"
#include "views/themes/theme_manager.h"

#include "arc_fill_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

ArcFillIndicator::ArcFillIndicator(uint32_t id, std::shared_ptr<Frame> parent)
    : IndicatorBase(id, parent) { }

int ArcFillIndicator::DoRender() {
    auto lv_obj = Create(container_->GetObject(), static_cast<int32_t>(range_start_), static_cast<int32_t>(range_end_));
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int ArcFillIndicator::ApplyTheme() {
    auto theme = ThemeManager::GetInstance().GetCurrentTheme();

    lv_obj_set_style_arc_color(lv_arc_, theme->GetSecondaryColor().ToLvColor(), LV_PART_INDICATOR | LV_STATE_DEFAULT);

    return 0;
}

lv_obj_t* ArcFillIndicator::Create(lv_obj_t* parent, int32_t range_start, int32_t range_end) {
    lv_arc_ = lv_arc_create(parent);

    lv_arc_set_range(lv_arc_, range_start, range_end);
    lv_obj_set_width(lv_arc_, lv_pct(100));
    lv_obj_set_height(lv_arc_, lv_pct(100));
    lv_obj_set_align(lv_arc_, LV_ALIGN_CENTER);
    lv_obj_remove_flag(lv_arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_value(lv_arc_, range_start);
    // lv_obj_set_style_arc_rounded(ui_arc, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    // lv_obj_set_style_bg_color(lv_arc_, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_arc_, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // lv_obj_set_style_arc_color(lv_arc_, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(lv_arc_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    return lv_arc_;
}

void ArcFillIndicator::UpdateIndicator(int32_t value) {
    lv_arc_set_value(lv_arc_, value);
    value_ = static_cast<float>(value);
}

} // namespace eerie_leap::views::widgets::indicators

#include <zephyr/kernel.h>

#include "views/widgets/indicators/indicator_base.h"
#include "views/themes/theme_manager.h"

#include "horizontal_bar_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

HorizontalBarIndicator::HorizontalBarIndicator(uint32_t id, std::shared_ptr<Frame> parent)
    : IndicatorBase(id, parent) { }

int HorizontalBarIndicator::DoRender() {
    auto lv_obj = Create(container_->GetObject(), static_cast<int32_t>(range_start_), static_cast<int32_t>(range_end_));
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int HorizontalBarIndicator::ApplyTheme() {
    auto theme = ThemeManager::GetInstance().GetCurrentTheme();

    lv_obj_set_style_bg_color(lv_bar_, theme->GetSecondaryColor().ToLvColor(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(lv_bar_, theme->GetSecondaryColor().ToLvOpa(), LV_PART_INDICATOR);

    return 0;
}

lv_obj_t* HorizontalBarIndicator::Create(lv_obj_t* parent, int32_t range_start, int32_t range_end) {
    lv_bar_ = lv_bar_create(parent);
    lv_obj_remove_style_all(lv_bar_);

    lv_obj_set_width(lv_bar_, lv_pct(100));
    lv_obj_set_height(lv_bar_, lv_pct(100));
    lv_obj_set_align(lv_bar_, LV_ALIGN_CENTER);
    lv_obj_remove_flag(lv_bar_, LV_OBJ_FLAG_CLICKABLE);
    lv_bar_set_range(lv_bar_, range_start, range_end);
    lv_bar_set_value(lv_bar_, range_start, LV_ANIM_OFF);

    // Background color - LV_PART_MAIN
    // Bar color - LV_PART_INDICATOR
    lv_obj_set_style_bg_opa(lv_bar_, 0, LV_PART_MAIN);

    return lv_bar_;
}

void HorizontalBarIndicator::UpdateIndicator(float value) {
    lv_bar_set_value(lv_bar_, static_cast<int32_t>(value), LV_ANIM_OFF);
}

void HorizontalBarIndicator::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    IndicatorBase::Configure(configuration);
}

} // namespace eerie_leap::views::widgets::indicators

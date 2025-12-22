#include "views/themes/theme_manager.h"

#include "digital_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

DigitalIndicator::DigitalIndicator(uint32_t id, std::shared_ptr<Frame> parent)
    : IndicatorBase(id, parent) { }

int DigitalIndicator::DoRender() {
    auto lv_obj = Create(container_);
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int DigitalIndicator::ApplyTheme() {
    auto theme = ThemeManager::GetInstance().GetCurrentTheme();

    lv_obj_set_style_text_font(lv_label_, theme->GetPrimaryFontLarge().ToLvFont(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_label_, theme->GetPrimaryColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(lv_label_, theme->GetPrimaryColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

lv_obj_t* DigitalIndicator::Create(std::shared_ptr<Frame> parent) {
    lv_label_ = lv_label_create(parent->GetObject());

    lv_obj_set_width(lv_label_, LV_SIZE_CONTENT);
    lv_obj_set_height(lv_label_, LV_SIZE_CONTENT);
    lv_label_set_text(lv_label_, "0");

    // Alignment
    lv_obj_center(lv_label_);
    lv_obj_set_align(lv_label_, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(lv_label_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    return lv_label_;
}

void DigitalIndicator::UpdateIndicator(float value) {
    char value_str[8];
    snprintf(value_str, sizeof(value_str), "%.*f", value_precision_, value);

    lv_label_set_text(lv_label_, value_str);
}

void DigitalIndicator::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    IndicatorBase::Configure(configuration);

    value_precision_ = GetConfigValue<int>(
        configuration->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::VALUE_PRECISION),
        0);
}

} // namespace eerie_leap::views::widgets::indicators

#include <algorithm>
#include <cstdio>
#include <optional>
#include <utility>

#include "domain/ui_domain/models/widget_property.h"

#include "setting_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

using eerie_leap::domain::settings_domain::utilities::ToSettingNumber;

SettingIndicator::SettingIndicator(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : SettingWidgetBase(id, std::move(parent), std::move(context)) {}

void SettingIndicator::RegisterProperties(WidgetPropertyStore& store) {
    SettingWidgetBase::RegisterProperties(store);

    store.Register(WidgetPropertyType::LABEL, ConfigValue { std::pmr::string { } }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::UNIT, ConfigValue { std::pmr::string { } }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::VALUE_PRECISION, ConfigValue { 0 }, PropertyChangeEffect::Repaint);
}

void SettingIndicator::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    switch(type) {
        case WidgetPropertyType::LABEL:
            label_ = ConfigValueAs<std::pmr::string>(value, "");
            break;

        case WidgetPropertyType::UNIT:
            unit_ = ConfigValueAs<std::pmr::string>(value, "");
            break;

        // Bounded because it reaches snprintf's "%.*f" precision.
        case WidgetPropertyType::VALUE_PRECISION:
            value_precision_ = std::clamp(ConfigValueAs<int>(value, 0), 0, max_value_precision_);
            break;

        default:
            SettingWidgetBase::OnPropertyChanged(type, value);
            break;
    }
}

int SettingIndicator::DoRender() {
    lv_label_ = lv_label_create(container_->GetObject());

    lv_obj_set_width(lv_label_, LV_SIZE_CONTENT);
    lv_obj_set_height(lv_label_, LV_SIZE_CONTENT);
    lv_obj_center(lv_label_);
    lv_obj_set_style_text_align(lv_label_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    UpdateText();

    container_->SetChild(std::make_shared<Frame>(Frame::Create(lv_label_).Build()));

    return 0;
}

int SettingIndicator::ApplyTheme(const ITheme& theme) {
    lv_obj_set_style_text_font(lv_label_, theme.GetPrimaryFont().ToLvFont(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_label_, theme.GetPrimaryColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(lv_label_, theme.GetPrimaryColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

void SettingIndicator::OnSettingChanged() {
    UpdateText();
}

void SettingIndicator::UpdateText() {
    if(lv_label_ == nullptr)
        return;

    std::optional<double> number;
    if(auto value = GetSettingValue())
        number = ToSettingNumber(*value);

    char text[128];
    if(number.has_value())
        snprintf(text, sizeof(text), "%s%s%.*f%s",
            label_.c_str(),
            label_.empty() ? "" : " ",
            value_precision_,
            *number,
            unit_.c_str());
    else
        snprintf(text, sizeof(text), "%s", label_.c_str());

    lv_label_set_text(lv_label_, text);
}

} // namespace eerie_leap::views::widgets::indicators

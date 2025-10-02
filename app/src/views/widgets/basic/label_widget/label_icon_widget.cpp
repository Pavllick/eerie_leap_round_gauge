#include "views/utilitites/positioning_helpers.h"
#include "views/themes/theme_manager.h"

#include "label_icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

LabelIconWidget::LabelIconWidget(uint32_t id, std::shared_ptr<Frame> parent)
    : WidgetBase(id, parent), is_active_(false) { }

int LabelIconWidget::DoRender() {
    auto lv_obj = Create(container_->GetObject());
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int LabelIconWidget::ApplyTheme() {
    auto theme = ThemeManager::GetInstance().GetCurrentTheme();

    if(is_active_) {
        lv_obj_set_style_bg_color(lv_icon_, theme->GetAccentColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_icon_, theme->GetAccentColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_color(lv_icon_, theme->GetInactiveColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_icon_, theme->GetInactiveColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_obj_set_style_text_color(lv_label_, theme->GetPrimaryColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(lv_label_, theme->GetPrimaryColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lv_label_, theme->GetPrimaryFont().ToLvFont(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_update_layout(lv_label_);

    lv_coord_t label_width = lv_obj_get_width(lv_label_);
    lv_coord_t label_height = lv_obj_get_height(lv_label_);

    if(label_width == 0 || label_height == 0)
        throw std::runtime_error("IconWidget: Label dimensions are invalid.");

    int32_t icon_width = label_width + label_height * 0.75;
    int32_t icon_height = label_height + label_height / 4;

    lv_obj_set_width(lv_icon_, icon_width);
    lv_obj_set_height(lv_icon_, icon_height);

    lv_obj_set_style_radius(lv_icon_, icon_height / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_pivot_x(lv_icon_, icon_width / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_pivot_y(lv_icon_, icon_height / 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_x(lv_icon_, position_x_);
    lv_obj_set_y(lv_icon_, position_y_);

    return 0;
}

lv_obj_t* LabelIconWidget::Create(lv_obj_t* parent) {
    lv_icon_ = lv_obj_create(parent);
    lv_obj_set_align(lv_icon_, LV_ALIGN_CENTER);
    lv_obj_remove_flag(lv_icon_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(lv_icon_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(lv_icon_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_label_ = lv_label_create(lv_icon_);
    lv_obj_set_width(lv_label_, LV_SIZE_CONTENT);
    lv_obj_set_height(lv_label_, LV_SIZE_CONTENT);
    lv_obj_set_align(lv_label_, LV_ALIGN_CENTER);
    lv_label_set_text(lv_label_, label.c_str());

    return lv_icon_;
}

void LabelIconWidget::Configure(const WidgetConfiguration& config) {
    WidgetBase::Configure(config);

    position_x_ = GetConfigValue<int>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X),
        0);

    position_y_ = GetConfigValue<int>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y),
        0);

    label = GetConfigValue<std::string>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::LABEL),
        "");

    is_active_ = GetConfigValue<bool>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE),
        false);

    auto event_type_raw = GetConfigValue<int>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE),
        0);
    auto event_type = static_cast<UiEventType>(event_type_raw);

    if(event_type != UiEventType::NONE) {
        auto event_subscription = UiEventBus::GetInstance().Subscribe(
            event_type,
            [this](const UiEvent& event) {
                if (auto it = event.payload.find(UiPayloadType::VALUE); it != event.payload.end()) {
                    if (auto* value = std::get_if<bool>(&it->second)) {
                        is_active_ = *value;
                        ApplyTheme();
                    }
                }
            }
        );

        if(event_subscription)
            subscriptions_.push_back(std::move(*event_subscription));
    }
}

} // namespace eerie_leap::views::widgets::basic

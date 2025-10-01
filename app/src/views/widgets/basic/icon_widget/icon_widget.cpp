#include "views/utilitites/positioning_helpers.h"
#include "views/themes/theme_manager.h"

#include "icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

IconWidget::IconWidget(uint32_t id, std::shared_ptr<Frame> parent)
    : WidgetBase(id, parent), is_active_(false) { }

int IconWidget::DoRender() {
    auto lv_obj = Create(container_->GetObject());
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int IconWidget::ApplyTheme() {
    auto theme = ThemeManager::GetInstance().GetCurrentTheme();

    if(is_active_) {
        lv_obj_set_style_bg_color(ui_icon_, theme->GetAccentColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_icon_, theme->GetAccentColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_color(ui_icon_, theme->GetInactiveColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_icon_, theme->GetInactiveColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_obj_set_style_text_color(ui_label_, theme->GetPrimaryColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_label_, theme->GetPrimaryColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_label_, theme->GetPrimaryFont().ToLvFont(), LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

// TODO: Make parameters configurable
lv_obj_t* IconWidget::Create(lv_obj_t* parent) {
    // TODO: config width to account for label length
    int32_t width = 48;
    int32_t height = 26;

    ui_icon_ = lv_obj_create(parent);
    lv_obj_set_width(ui_icon_, width);
    lv_obj_set_height(ui_icon_, height);
    lv_obj_set_align(ui_icon_, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_icon_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_icon_, 13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_icon_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_icon_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_pivot_x(ui_icon_, width / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_pivot_y(ui_icon_, height / 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_label_ = lv_label_create(ui_icon_);
    lv_obj_set_width(ui_label_, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_, label.c_str());

    PositioningHelpers::PlaceObjectOnCircle(ui_icon_, 0, 0, 233 - height / 2 - 2, 90.0f + 34.0f);

    return ui_icon_;
}

void IconWidget::Configure(const WidgetConfiguration& config) {
    WidgetBase::Configure(config);

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
        auto logging_subscription = UiEventBus::GetInstance().Subscribe(
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

        if(logging_subscription)
            subscriptions_.push_back(std::move(*logging_subscription));
    }
}

} // namespace eerie_leap::views::widgets::basic

#include "views/utilitites/positioning_helpers.h"

#include "icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

IconWidget::IconWidget(uint32_t id, std::shared_ptr<Frame> parent)
    : WidgetBase(id, parent) { }

int IconWidget::DoRender() {
    auto lv_obj = Create(container_->GetObject());
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

// TODO: Make parameters configurable
lv_obj_t* IconWidget::Create(lv_obj_t* parent) {
    // TODO: config width to account for label length
    int32_t width = 48;
    int32_t height = 26;

    auto ui_panel = lv_obj_create(parent);
    lv_obj_set_width(ui_panel, width);
    lv_obj_set_height(ui_panel, height);
    lv_obj_set_align(ui_panel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_panel, 13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_panel, lv_color_hex(0xF56060), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_pivot_x(ui_panel, width / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_pivot_y(ui_panel, height / 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    auto ui_label = lv_label_create(ui_panel);
    lv_obj_set_width(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label, label.c_str());
    lv_obj_set_style_text_color(ui_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    PositioningHelpers::PlaceObjectOnCircle(ui_panel, 0, 0, 233 - height / 2 - 2, 90.0f + 34.0f);

    return ui_panel;
}

void IconWidget::Configure(const WidgetConfiguration& config) {
    WidgetBase::Configure(config);

    label = GetConfigValue<std::string>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::LABEL),
        "");

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
                    if (auto* value = std::get_if<bool>(&it->second))
                        SetVisibility(*value);
                }
            }
        );

        if(logging_subscription)
            subscriptions_.push_back(std::move(*logging_subscription));
    }
}

} // namespace eerie_leap::views::widgets::basic

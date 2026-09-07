#include <utility>

#include <zephyr/logging/log.h>

#include "domain/ui_domain/models/widget_property.h"

#include "button_control.h"

namespace eerie_leap::views::widgets::controls {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

LOG_MODULE_REGISTER(button_control_logger);

// A button carries no setting, so an unset target means "no navigation".
static constexpr int32_t no_target = -1;

ButtonControl::ButtonControl(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : ControlBase(id, std::move(parent), std::move(context), false) {}

void ButtonControl::RegisterProperties(WidgetPropertyStore& store) const {
    ControlBase::RegisterProperties(store);

    store.Register(WidgetPropertyType::LABEL, ConfigValue { std::pmr::string { } }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::TARGET_SCREEN_GROUP, ConfigValue { no_target }, PropertyChangeEffect::None);
    store.Register(
        WidgetPropertyType::NAVIGATION_INTENT,
        ConfigValue { static_cast<int>(NavigationIntent::None) },
        PropertyChangeEffect::None);
}

void ButtonControl::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    switch(type) {
        case WidgetPropertyType::LABEL:
            label_ = ConfigValueAs<std::pmr::string>(value, "");
            break;

        case WidgetPropertyType::TARGET_SCREEN_GROUP: {
            auto target = ConfigValueAs<int>(value, no_target);

            if(target >= 0)
                target_id_ = static_cast<uint32_t>(target);

            break;
        }

        case WidgetPropertyType::NAVIGATION_INTENT:
            intent_ = static_cast<NavigationIntent>(
                ConfigValueAs<int>(value, static_cast<int>(NavigationIntent::None)));
            break;

        default:
            ControlBase::OnPropertyChanged(type, value);
            break;
    }
}

int ButtonControl::DoRender() {
    lv_button_ = lv_button_create(container_->GetObject());

    lv_obj_set_size(lv_button_, lv_pct(100), lv_pct(100));
    lv_obj_center(lv_button_);

    lv_label_ = lv_label_create(lv_button_);
    lv_label_set_text(lv_label_, label_.c_str());
    lv_obj_center(lv_label_);

    AttachEvents(lv_button_, { LV_EVENT_CLICKED });

    container_->SetChild(std::make_shared<Frame>(Frame::Create(lv_button_).Build()));

    return 0;
}

int ButtonControl::ApplyTheme(const ITheme& theme) {
    lv_obj_set_style_bg_color(lv_button_, theme.GetSurfaceColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_button_, theme.GetSurfaceColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(lv_button_, theme.GetAccentColor().ToLvColor(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(lv_button_, theme.GetAccentColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_set_style_text_font(lv_label_, theme.GetPrimaryFont().ToLvFont(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_label_, theme.GetPrimaryColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(lv_label_, theme.GetPrimaryColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

void ButtonControl::OnControlEvent(lv_event_code_t code) {
    if(code != LV_EVENT_CLICKED || context_.navigation_service == nullptr)
        return;

    auto navigation = ResolveNavigation();
    if(!navigation.has_value())
        return;

    // Publishes an event; the view change itself happens on the event bus thread.
    if(context_.navigation_service->Handle(navigation->first, navigation->second) != 0)
        LOG_WRN("Widget %u failed navigation intent %u.", id_, static_cast<uint32_t>(navigation->first));
}

std::optional<std::pair<NavigationIntent, uint32_t>> ButtonControl::ResolveNavigation() const {
    // A button configured with only a target predates NAVIGATION_INTENT.
    auto intent = intent_ == NavigationIntent::None && target_id_.has_value()
        ? NavigationIntent::GoToGroup
        : intent_;

    switch(intent) {
        case NavigationIntent::GoToGroup:
        case NavigationIntent::ShowOverlay:
            if(target_id_.has_value())
                return std::pair { intent, *target_id_ };

            return std::nullopt;

        case NavigationIntent::None:
            return std::nullopt;

        default:
            return std::pair { intent, uint32_t { 0 } };
    }
}

} // namespace eerie_leap::views::widgets::controls

#include <utility>

#include "toggle_control.h"

namespace eerie_leap::views::widgets::controls {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

using eerie_leap::domain::ui_domain::services::ToSettingBoolean;

ToggleControl::ToggleControl(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : ControlBase(id, std::move(parent), std::move(context), true) {}

int ToggleControl::DoRender() {
    lv_switch_ = lv_switch_create(container_->GetObject());

    lv_obj_center(lv_switch_);

    AttachEvents(lv_switch_, { LV_EVENT_VALUE_CHANGED });

    SyncFromSetting();

    container_->SetChild(std::make_shared<Frame>(Frame::Create(lv_switch_).Build()));

    return 0;
}

int ToggleControl::ApplyTheme(const ITheme& theme) {
    lv_obj_set_style_bg_color(lv_switch_, theme.GetSurfaceColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_switch_, theme.GetSurfaceColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(lv_switch_, theme.GetPrimaryColor().ToLvColor(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(lv_switch_, theme.GetPrimaryColor().ToLvOpa(), LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(lv_switch_, theme.GetAccentColor().ToLvColor(), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_switch_, theme.GetAccentColor().ToLvOpa(), LV_PART_KNOB | LV_STATE_DEFAULT);

    return 0;
}

void ToggleControl::OnControlEvent(lv_event_code_t code) {
    if(code != LV_EVENT_VALUE_CHANGED)
        return;

    bool is_checked = lv_obj_has_state(lv_switch_, LV_STATE_CHECKED);

    // A toggle has no drag, so applying and persisting happen together.
    if(SetSettingValue(ConfigValue { is_checked }) != 0) {
        SyncFromSetting();
        return;
    }

    CommitSetting();
}

void ToggleControl::OnSettingChanged() {
    SyncFromSetting();
}

void ToggleControl::SyncFromSetting() {
    if(lv_switch_ == nullptr)
        return;

    auto value = GetSettingValue();
    if(!value.has_value())
        return;

    auto is_checked = ToSettingBoolean(*value);
    if(!is_checked.has_value())
        return;

    if(*is_checked)
        lv_obj_add_state(lv_switch_, LV_STATE_CHECKED);
    else
        lv_obj_remove_state(lv_switch_, LV_STATE_CHECKED);
}

} // namespace eerie_leap::views::widgets::controls

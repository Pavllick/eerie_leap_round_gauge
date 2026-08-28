#include <algorithm>
#include <cmath>
#include <utility>

#include "domain/ui_domain/models/widget_property.h"

#include "slider_control.h"

namespace eerie_leap::views::widgets::controls {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

using eerie_leap::domain::settings_domain::utilities::ToSettingNumber;

SliderControl::SliderControl(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : ControlBase(id, std::move(parent), std::move(context), true) {}

void SliderControl::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    ControlBase::Configure(configuration);

    configured_step_ = GetConfigValue<double>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::STEP),
        0);

    ApplyRange();
}

void SliderControl::OnRangeResolved() {
    ApplyRange();
}

void SliderControl::ApplyRange() {
    double range_step = range_.has_value() && range_->step > 0 ? range_->step : 1.0;
    step_ = configured_step_ > 0 ? configured_step_ : range_step;

    // Ordered up front so a binding that declares its bounds the wrong way round
    // cannot reach std::clamp with lo > hi.
    min_ = range_.has_value() ? std::min(range_->min, range_->max) : 0;
    max_ = range_.has_value() ? std::max(range_->min, range_->max) : 0;

    step_count_ = std::max(static_cast<int32_t>(std::lround((max_ - min_) / step_)), 1);

    if(lv_slider_ != nullptr)
        lv_slider_set_range(lv_slider_, 0, step_count_);
}

int32_t SliderControl::ToIndex(double value) const {
    auto index = static_cast<int32_t>(std::lround((value - min_) / step_));

    return std::clamp(index, 0, step_count_);
}

double SliderControl::ToValue(int32_t index) const {
    return std::clamp(min_ + (index * step_), min_, max_);
}

int SliderControl::DoRender() {
    lv_slider_ = lv_slider_create(container_->GetObject());

    lv_obj_set_width(lv_slider_, lv_pct(100));
    lv_obj_center(lv_slider_);
    lv_slider_set_range(lv_slider_, 0, step_count_);

    AttachEvents(lv_slider_, { LV_EVENT_VALUE_CHANGED, LV_EVENT_RELEASED });

    SyncFromSetting();

    container_->SetChild(std::make_shared<Frame>(Frame::Create(lv_slider_).Build()));

    return 0;
}

int SliderControl::ApplyTheme(const ITheme& theme) {
    lv_obj_set_style_bg_color(lv_slider_, theme.GetSurfaceColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_slider_, theme.GetSurfaceColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(lv_slider_, theme.GetPrimaryColor().ToLvColor(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_slider_, theme.GetPrimaryColor().ToLvOpa(), LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(lv_slider_, theme.GetAccentColor().ToLvColor(), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_slider_, theme.GetAccentColor().ToLvOpa(), LV_PART_KNOB | LV_STATE_DEFAULT);

    return 0;
}

void SliderControl::OnControlEvent(lv_event_code_t code) {
    if(code == LV_EVENT_VALUE_CHANGED) {
        // Writing without a resolved range would send the binding a value derived
        // from the placeholder 0..0 bounds.
        if(!range_.has_value())
            return;

        if(SetSettingValue(ConfigValue { ToValue(lv_slider_get_value(lv_slider_)) }) != 0)
            SyncFromSetting();

        return;
    }

    // Applied on every drag tick, persisted only once the finger is lifted. The
    // sync settles the knob onto whatever the binding actually took, since the
    // publications raised during the drag were ignored.
    if(code == LV_EVENT_RELEASED) {
        CommitSetting();
        SyncFromSetting();
    }
}

void SliderControl::OnSettingChanged() {
    // The publication this widget just caused would otherwise snap the knob away
    // from the finger between drag ticks.
    if(lv_slider_ != nullptr && lv_obj_has_state(lv_slider_, LV_STATE_PRESSED))
        return;

    SyncFromSetting();
}

void SliderControl::SyncFromSetting() {
    if(lv_slider_ == nullptr)
        return;

    auto value = GetSettingValue();
    if(!value.has_value())
        return;

    auto number = ToSettingNumber(*value);
    if(!number.has_value())
        return;

    lv_slider_set_value(lv_slider_, ToIndex(*number), LV_ANIM_OFF);
}

} // namespace eerie_leap::views::widgets::controls

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

SliderControl::SliderControl(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : ControlBase(id, std::move(parent), std::move(context), true) {}

void SliderControl::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    switch(type) {
        case WidgetPropertyType::MIN_VALUE:
        case WidgetPropertyType::MAX_VALUE:
        case WidgetPropertyType::STEP:
            ControlBase::OnPropertyChanged(type, value);
            ApplyRange();
            break;

        case WidgetPropertyType::VALUE:
            ControlBase::OnPropertyChanged(type, value);
            UpdateSlider();
            break;

        default:
            ControlBase::OnPropertyChanged(type, value);
            break;
    }
}

// Reads the store rather than caching each bound value: the three arrive as separate events and
// only make sense together.
void SliderControl::ApplyRange() {
    double low = properties_->GetAs<double>(WidgetPropertyType::MIN_VALUE, 0);
    double high = properties_->GetAs<double>(WidgetPropertyType::MAX_VALUE, 0);
    double step = properties_->GetAs<double>(WidgetPropertyType::STEP, 0);

    // Ordered up front so an owner that declares its bounds the wrong way round cannot reach
    // std::clamp with lo > hi.
    min_ = std::min(low, high);
    max_ = std::max(low, high);
    step_ = step > 0 ? step : 1.0;

    step_count_ = std::max(static_cast<int32_t>(std::lround((max_ - min_) / step_)), 1);

    if(lv_slider_ != nullptr)
        lv_slider_set_range(lv_slider_, 0, step_count_);

    UpdateSlider();
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

    UpdateSlider();

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
        // Asking without a range would send a value derived from the placeholder 0..0 bounds.
        if(max_ <= min_)
            return;

        RequestSettingValue(ConfigValue { ToValue(lv_slider_get_value(lv_slider_)) });

        return;
    }

    // Requests go out on every drag tick; the settle happens once the finger is lifted, when
    // the knob is free to jump to whatever the owner actually took.
    if(code == LV_EVENT_RELEASED)
        UpdateSlider();
}

void SliderControl::UpdateSlider() {
    if(lv_slider_ == nullptr)
        return;

    // The value this widget just asked for would otherwise snap the knob away from the finger
    // between drag ticks.
    if(lv_obj_has_state(lv_slider_, LV_STATE_PRESSED))
        return;

    lv_slider_set_value(lv_slider_, ToIndex(properties_->GetAs<double>(WidgetPropertyType::VALUE, 0)), LV_ANIM_OFF);
}

} // namespace eerie_leap::views::widgets::controls

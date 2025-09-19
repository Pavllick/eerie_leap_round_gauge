#include <zephyr/kernel.h>

#include "utilities/type/config_value.h"
#include "views/widgets/utilitites/frame.h"
#include "views/widgets/configuration/widget_property.h"

#include "arc_fill_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::views::widgets::configuration;
using namespace eerie_leap::views::widgets::utilitites;

ArcFillIndicator::ArcFillIndicator(uint32_t id) : id_(id), state_() {
    auto container = std::make_shared<Frame>();
    container->Build();
    state_.container = std::move(container);

    value_change_animation_ = CreateValueChangeAnimation();
}

int ArcFillIndicator::Render() {
    state_.lv_obj = Create(state_.container->GetObject(), static_cast<int32_t>(state_.range_start), static_cast<int32_t>(state_.range_end));

    return 0;
}

lv_obj_t* ArcFillIndicator::Create(lv_obj_t* parent, int32_t range_start, int32_t range_end) {
    lv_obj_t* ui_arc = lv_arc_create(parent);

    lv_arc_set_range(ui_arc, range_start, range_end);
    lv_obj_set_width(ui_arc, lv_pct(100));
    lv_obj_set_height(ui_arc, lv_pct(100));
    lv_obj_set_align(ui_arc, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_value(ui_arc, range_start);
    lv_obj_set_style_arc_opa(ui_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_arc, lv_color_hex(0x1BBE5F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    return ui_arc;
}

bool ArcFillIndicator::IsAnimationEnabled() const {
    return animation_enabled_;
}

void ArcFillIndicator::UpdateIndicator(void* obj, int32_t value) {
    auto* state = (IndicatorState*)obj;

    lv_arc_set_value(state->lv_obj, value);
    state->value = static_cast<float>(value);
}

lv_anim_t ArcFillIndicator::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, &state_);
    lv_anim_set_exec_cb(&anim, UpdateIndicator);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

void ArcFillIndicator::ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_start(&anim);
}

void ArcFillIndicator::Update(float value) {
    if(animation_enabled_)
        ValueChangeAnimation(value_change_animation_, state_.range_end - state_.range_start, state_.value, static_cast<int32_t>(value));
    else
        UpdateIndicator(&state_, static_cast<int>(value));
}

const IndicatorState* ArcFillIndicator::GetState() const {
    return &state_;
}

uint32_t ArcFillIndicator::GetId() const {
    return id_;
}

std::optional<uint32_t> ArcFillIndicator::GetSensorId() const {
    return sensor_id_;
}

void ArcFillIndicator::Configure(const WidgetConfiguration& config) {
    configuration_ = config;

    animation_enabled_ = config.is_animation_enabled;

    auto sensor_id_str = GetConfigValue<std::string>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::SENSOR_ID),
        "");

    if(!sensor_id_str.empty()) {
        sensor_id_ = std::stoul(sensor_id_str);
    }

    state_.range_start = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MIN_VALUE),
        0);
    state_.range_end = GetConfigValue<int>(
        config.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::MAX_VALUE),
        100);
    state_.value = state_.range_start;
}

WidgetConfiguration ArcFillIndicator::GetConfiguration() const {
    return configuration_;
}

WidgetPosition ArcFillIndicator::GetPosition() const {
    return position_;
}

void ArcFillIndicator::SetPosition(const WidgetPosition& pos) {
    position_ = pos;
}

WidgetSize ArcFillIndicator::GetSize() const {
    return size_;
}

void ArcFillIndicator::SetSize(const WidgetSize& size) {
    size_ = size;
}

} // namespace eerie_leap::views::widgets::indicators

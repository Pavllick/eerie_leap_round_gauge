#include <string>

#include "utilities/type/config_value.h"
#include "views/widgets/configuration/widget_property.h"
#include "views/widgets/utilitites/frame.h"

#include "digital_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::views::widgets::configuration;
using namespace eerie_leap::views::widgets::utilitites;

DigitalIndicator::DigitalIndicator(uint32_t id) : id_(id), state_() {
    auto container = std::make_shared<Frame>();
    container->Build();
    state_.container = std::move(container);

    value_change_animation_ = CreateValueChangeAnimation();
}

int DigitalIndicator::Render() {
    state_.lv_obj = Create(state_.container->GetObject());

    return 0;
}

lv_obj_t* DigitalIndicator::Create(lv_obj_t* parent) {
    auto ui_label = lv_label_create(parent);

    lv_obj_set_width(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label, "0");
    lv_obj_set_style_text_font(ui_label, &lv_font_unscii_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_scale(ui_label, 1500, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_label, -65);
    lv_obj_set_y(ui_label, -55);

    return ui_label;
}

bool DigitalIndicator::IsAnimationEnabled() const {
    return animation_enabled_;
}

void DigitalIndicator::UpdateIndicator(void* obj, int32_t value) {
    auto* state = (IndicatorState*)obj;

    lv_label_set_text(state->lv_obj, std::to_string(value).c_str());
    state->value = static_cast<float>(value);
}

lv_anim_t DigitalIndicator::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, &state_);
    lv_anim_set_exec_cb(&anim, UpdateIndicator);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

void DigitalIndicator::ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_start(&anim);
}

void DigitalIndicator::Update(float value) {
    if(animation_enabled_)
        ValueChangeAnimation(value_change_animation_, state_.range_end - state_.range_start, state_.value, static_cast<int32_t>(value));
    else
        UpdateIndicator(&state_, static_cast<int>(value));
}

void DigitalIndicator::Configure(const WidgetConfiguration& config) {
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

const IndicatorState* DigitalIndicator::GetState() const {
    return &state_;
}

uint32_t DigitalIndicator::GetId() const {
    return id_;
}

std::optional<uint32_t> DigitalIndicator::GetSensorId() const {
    return sensor_id_;
}

WidgetConfiguration DigitalIndicator::GetConfiguration() const {
    return configuration_;
}

WidgetPosition DigitalIndicator::GetPosition() const {
    return position_;
}

void DigitalIndicator::SetPosition(const WidgetPosition& pos) {
    position_ = pos;
}

WidgetSize DigitalIndicator::GetSize() const {
    return size_;
}

void DigitalIndicator::SetSize(const WidgetSize& size) {
    size_ = size;
}

} // namespace eerie_leap::views::widgets::indicators

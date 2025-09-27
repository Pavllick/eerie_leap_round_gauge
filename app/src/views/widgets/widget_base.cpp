#include "domain/ui_domain/models/widget_property.h"

#include "widget_base.h"

namespace eerie_leap::views::widgets {

WidgetBase::WidgetBase(uint32_t id) : id_(id) {
    auto container = std::make_shared<Frame>();
    container->Build();
    container_ = std::move(container);
}

uint32_t WidgetBase::GetId() const {
    return id_;
}

bool WidgetBase::HasTag(WidgetTag tag) const {
    return tags_.contains(tag);
}

void WidgetBase::Configure(const WidgetConfiguration& config) {
    configuration_ = config;

    SetVisibility(IsVisible());

    UpdateTags(GetConfigValue<std::vector<int>>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::TAGS),
        {}));
}

WidgetConfiguration WidgetBase::GetConfiguration() const {
    return configuration_;
}

bool WidgetBase::UpdateProperty(const WidgetPropertyType property_type, const ConfigValue& value, bool force_update) {
    auto property_name = WidgetProperty::GetTypeName(property_type);

    if(!force_update && !configuration_.properties.contains(property_name))
        return false;

    configuration_.properties[property_name] = value;

    if(property_type == WidgetPropertyType::IS_VISIBLE)
        SetVisibility(std::get<bool>(value));

    return true;
}

void WidgetBase::UpdateTags(std::vector<int> tag_values) {
    tags_.clear();
    for(auto tag_value : tag_values)
        tags_.insert(static_cast<WidgetTag>(tag_value));
}

int WidgetBase::SetVisibility(bool is_visible) {
    if(is_visible)
        lv_obj_clear_flag(container_->GetObject(), LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(container_->GetObject(), LV_OBJ_FLAG_HIDDEN);

    return 0;
}

bool WidgetBase::IsVisible() const {
    return GetConfigValue<bool>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE),
        false);
}

bool WidgetBase::IsAnimated() const {
    return GetConfigValue<bool>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED),
        false);
}

WidgetPosition WidgetBase::GetPositionPx() const {
    return position_px_;
}

void WidgetBase::SetPositionPx(const WidgetPosition& position_px) {
    position_px_ = position_px;

    container_->SetXOffset(position_px.x, true)
        .SetYOffset(position_px.y, true);
}

WidgetSize WidgetBase::GetSizePx() const {
    return size_px_;
}

void WidgetBase::SetSizePx(const WidgetSize& size_px) {
    size_px_ = size_px;

    container_->SetHeight(size_px.height, true)
        .SetWidth(size_px.width, true);
}

} // namespace eerie_leap::views::widgets

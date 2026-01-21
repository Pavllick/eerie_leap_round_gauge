#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "domain/ui_domain/models/widget_property.h"

#include "widget_base.h"

namespace eerie_leap::views::widgets {

WidgetBase::WidgetBase(uint32_t id, std::shared_ptr<Frame> parent)
    : id_(id), parent_(std::move(parent)) {

    container_ = std::make_shared<Frame>(Frame::CreateWrapped(parent_->GetObject())
        .SetWidth(100, false)
        .SetHeight(100, false)
        .Build());
}

WidgetBase::~WidgetBase() {
    for(auto& subscription : subscriptions_)
        UiEventBus::GetInstance().Unsubscribe(subscription);
}

void WidgetBase::SetAssetsManager(std::shared_ptr<UiAssetsManager> ui_assets_manager) {
    ui_assets_manager_ = std::move(ui_assets_manager);
}

uint32_t WidgetBase::GetId() const {
    return id_;
}

void WidgetBase::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    configuration_ = std::move(configuration);

    SetVisibility(IsVisible());
}

std::shared_ptr<WidgetConfiguration> WidgetBase::GetConfiguration() const {
    return configuration_;
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
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_VISIBLE),
        false);
}

bool WidgetBase::IsSmoothed() const {
    return GetConfigValue<bool>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_SMOOTHED),
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

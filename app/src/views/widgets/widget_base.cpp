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

void WidgetBase::Configure(const WidgetConfiguration& config) {
    configuration_ = config;
    is_animation_enabled_ = config.is_animation_enabled;
}

WidgetConfiguration WidgetBase::GetConfiguration() const {
    return configuration_;
}

bool WidgetBase::IsAnimationEnabled() const {
    return is_animation_enabled_;
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

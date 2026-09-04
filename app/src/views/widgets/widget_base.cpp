#include <algorithm>
#include <exception>

#include <zephyr/logging/log.h>

#include "domain/ui_domain/models/widget_property.h"

#include "widget_base.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;

LOG_MODULE_REGISTER(widget_base_logger);

WidgetBase::WidgetBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : id_(id), properties_(std::make_shared<WidgetPropertyStore>()), parent_(std::move(parent)),
    dispatch_guard_(std::make_shared<WidgetDispatchGuard>(this)),
    context_(std::move(context)) {

    container_ = std::make_shared<Frame>(Frame::CreateWrapped(parent_->GetObject())
        .SetWidth(100, false)
        .SetHeight(100, false)
        .Build());
}

WidgetBase::~WidgetBase() {
    DetachDispatch();

    // Before any other member goes: each entry unsubscribes as it is destroyed.
    subscriptions_.clear();
}

void WidgetBase::DetachDispatch() {
    dispatch_guard_->Detach();
}

void WidgetBase::AddSubscription(AnySubscription subscription) {
    if(subscription != nullptr)
        subscriptions_.push_back(std::move(subscription));
}

uint32_t WidgetBase::GetId() const {
    return id_;
}

bool WidgetBase::IsActive() const {
    return is_active_;
}

void WidgetBase::OnActivated() {
    is_active_ = true;

    // The store kept tracking while the group was hidden, so this is where the widget catches up.
    if(IsReady())
        ReplayProperties();
}

void WidgetBase::OnDeactivated() {
    is_active_ = false;
}

void WidgetBase::RegisterProperties(WidgetPropertyStore& store) {
    store.Register(WidgetPropertyType::IS_VISIBLE, ConfigValue { true }, PropertyChangeEffect::None);
    store.Register(WidgetPropertyType::IS_SMOOTHED, ConfigValue { false }, PropertyChangeEffect::None);
}

void WidgetBase::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    if(type == WidgetPropertyType::IS_VISIBLE)
        SetVisibility(ConfigValueAs<bool>(value, true));
}

void WidgetBase::OnConfigured() { }

void WidgetBase::RunEffect(PropertyChangeEffect effect) {
    if(effect == PropertyChangeEffect::None || !IsReady())
        return;

    container_->Invalidate();
}

void WidgetBase::ApplyProperty(WidgetPropertyType type, const ConfigValue& value) {
    if(!properties_->Set(type, value))
        return;

    auto effect = properties_->GetEffect(type);

    // LvglLock before the dispatch guard: ~WidgetBase runs under the LVGL lock and takes the
    // guard second, so the reverse order deadlocks.
    ScopedLvglLock lvgl_guard;

    dispatch_guard_->Dispatch([&] {
        if(!IsActive())
            return;

        OnPropertyChanged(type, value);
        RunEffect(effect);
    });
}

void WidgetBase::ReplayProperties() {
    auto strongest = PropertyChangeEffect::None;

    for(auto type : properties_->GetRegisteredTypes()) {
        OnPropertyChanged(type, properties_->Get(type));
        strongest = std::max(strongest, properties_->GetEffect(type));
    }

    RunEffect(strongest);
}

void WidgetBase::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    configuration_ = std::move(configuration);

    RegisterProperties(*properties_);

    // Visibility is still a field on the configuration rather than a persisted property.
    properties_->Set(WidgetPropertyType::IS_VISIBLE, ConfigValue { configuration_->is_visible });

    for(const auto& [key, value] : configuration_->properties) {
        try {
            if(!properties_->Set(WidgetProperty::GetType(key), value))
                LOG_WRN("Widget %u does not support property '%s'.", id_, key.c_str());
        } catch(const std::exception&) {
            LOG_WRN("Widget %u carries unknown property '%s'.", id_, key.c_str());
        }
    }

    ReplayProperties();
    OnConfigured();
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
    return properties_->GetAs<bool>(WidgetPropertyType::IS_VISIBLE, true);
}

bool WidgetBase::IsSmoothed() const {
    return properties_->GetAs<bool>(WidgetPropertyType::IS_SMOOTHED, false);
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

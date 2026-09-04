#include <cerrno>
#include <utility>

#include <zephyr/logging/log.h>

#include "utilities/string/string_helpers.h"

#include "domain/ui_domain/models/widget_property.h"

#include "views/widgets/event_bus_filters/setting_filter.h"

#include "setting_widget_base.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::settings_domain::event_bus;
using namespace eerie_leap::domain::ui_domain::models;

using eerie_leap::utilities::string::StringHelpers;
using eerie_leap::views::widgets::event_bus_filters::SettingFilter;

LOG_MODULE_REGISTER(setting_widget_base_logger);

SettingWidgetBase::SettingWidgetBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : WidgetBase(id, std::move(parent), std::move(context)) { }

void SettingWidgetBase::RegisterProperties(WidgetPropertyStore& store) {
    WidgetBase::RegisterProperties(store);

    store.Register(WidgetPropertyType::SETTING_ID, ConfigValue { std::pmr::string { } }, PropertyChangeEffect::None);
}

void SettingWidgetBase::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    if(type == WidgetPropertyType::SETTING_ID) {
        setting_id_ = ConfigValueAs<std::pmr::string>(value, "");
        return;
    }

    WidgetBase::OnPropertyChanged(type, value);
}

void SettingWidgetBase::OnConfigured() {
    WidgetBase::OnConfigured();

    if(setting_id_.empty())
        return;

    if(context_.settings_provider == nullptr) {
        LOG_WRN("Widget %u binds setting '%s' without a settings provider.", id_, setting_id_.c_str());
        return;
    }

    RefreshRange();

    SubscribeWhileActive(
        SettingsEventsChannel::GetInstance(),
        SettingsEventType::Changed,
        SettingFilter { StringHelpers::GetHash(setting_id_) },
        [this](const SettingsEventsChannel::EventMessage&) { OnSettingChanged(); });
}

void SettingWidgetBase::RefreshRange() {
    if(range_.has_value() || !HasSetting())
        return;

    range_ = context_.settings_provider->GetRange(setting_id_);
    if(!range_.has_value()) {
        LOG_WRN("Widget %u binds unregistered setting '%s'.", id_, setting_id_.c_str());
        return;
    }

    OnRangeResolved();
}

void SettingWidgetBase::OnActivated() {
    WidgetBase::OnActivated();

    RefreshRange();

    if(IsReady())
        OnSettingChanged();
}

void SettingWidgetBase::OnRangeResolved() { }

void SettingWidgetBase::OnSettingChanged() { }

bool SettingWidgetBase::HasSetting() const {
    return !setting_id_.empty() && context_.settings_provider != nullptr;
}

std::optional<ConfigValue> SettingWidgetBase::GetSettingValue() const {
    if(!HasSetting())
        return std::nullopt;

    return context_.settings_provider->Get(setting_id_);
}

int SettingWidgetBase::SetSettingValue(const ConfigValue& value) {
    if(!HasSetting())
        return -ENOENT;

    return context_.settings_provider->Set(setting_id_, value);
}

int SettingWidgetBase::CommitSetting() {
    if(!HasSetting())
        return -ENOENT;

    return context_.settings_provider->Commit(setting_id_);
}

} // namespace eerie_leap::views::widgets

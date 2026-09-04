#include <cstdint>
#include <utility>
#include <variant>

#include <zephyr/logging/log.h>

#include "utilities/reflection/caller_name.h"
#include "utilities/string/string_helpers.h"

#include "domain/settings_domain/event_bus/settings_events_channel.h"
#include "domain/ui_domain/models/widget_property.h"

#include "views/widgets/event_bus_filters/setting_filter.h"
#include "views/widgets/property_value_conversion.h"

#include "setting_widget_base.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::settings_domain::event_bus;
using namespace eerie_leap::domain::ui_domain::models;

using eerie_leap::utilities::reflection::GetCallerName;
using eerie_leap::utilities::string::StringHelpers;
using eerie_leap::views::widgets::event_bus_filters::SettingFilter;

LOG_MODULE_REGISTER(setting_widget_base_logger);

SettingWidgetBase::SettingWidgetBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context)
    : WidgetBase(id, std::move(parent), std::move(context)) { }

void SettingWidgetBase::RegisterProperties(WidgetPropertyStore& store) {
    WidgetBase::RegisterProperties(store);

    store.Register(WidgetPropertyType::SETTING_ID, ConfigValue { std::pmr::string { } }, PropertyChangeEffect::None);
    store.Register(WidgetPropertyType::VALUE, ConfigValue { 0.0 }, PropertyChangeEffect::Repaint);
    store.Register(WidgetPropertyType::MIN_VALUE, ConfigValue { 0.0 }, PropertyChangeEffect::Relayout);
    store.Register(WidgetPropertyType::MAX_VALUE, ConfigValue { 0.0 }, PropertyChangeEffect::Relayout);
    store.Register(WidgetPropertyType::STEP, ConfigValue { 0.0 }, PropertyChangeEffect::Relayout);
}

void SettingWidgetBase::OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) {
    if(type == WidgetPropertyType::SETTING_ID) {
        setting_id_ = ConfigValueAs<std::pmr::string>(value, "");
        setting_id_hash_ = setting_id_.empty() ? 0 : StringHelpers::GetHash(setting_id_);

        return;
    }

    WidgetBase::OnPropertyChanged(type, value);
}

void SettingWidgetBase::OnConfigured() {
    WidgetBase::OnConfigured();

    if(!HasSetting())
        return;

    SubscribeWhileActive(
        SettingsEventsChannel::GetInstance(),
        SettingsEventType::Changed,
        SettingFilter { setting_id_hash_ },
        [this](const SettingsEventsChannel::EventMessage& event) {
            auto it = event.payload.find(SettingsPayloadType::Value);
            if(it != event.payload.end())
                ApplyProperty(WidgetPropertyType::VALUE, ToConfigValue(it->second));
        });

    SubscribeWhileActive(
        SettingsEventsChannel::GetInstance(),
        SettingsEventType::RangeChanged,
        SettingFilter { setting_id_hash_ },
        [this](const SettingsEventsChannel::EventMessage& event) {
            auto min = event.payload.find(SettingsPayloadType::MinValue);
            if(min != event.payload.end())
                ApplyProperty(WidgetPropertyType::MIN_VALUE, ToConfigValue(min->second));

            auto max = event.payload.find(SettingsPayloadType::MaxValue);
            if(max != event.payload.end())
                ApplyProperty(WidgetPropertyType::MAX_VALUE, ToConfigValue(max->second));

            auto step = event.payload.find(SettingsPayloadType::Step);
            if(step != event.payload.end())
                ApplyProperty(WidgetPropertyType::STEP, ToConfigValue(step->second));
        });

    RequestSettingState();
}

void SettingWidgetBase::OnActivated() {
    WidgetBase::OnActivated();

    RequestSettingState();
}

bool SettingWidgetBase::HasSetting() const {
    return setting_id_hash_ != 0;
}

void SettingWidgetBase::RequestSettingValue(const ConfigValue& value) const {
    static constexpr auto caller = GetCallerName();

    if(!HasSetting())
        return;

    SettingsEventsChannel::GetInstance().PublishAsync({
        .source_id = caller.hash,
        .type = SettingsEventType::ChangeRequested,
        .payload = {
            { SettingsPayloadType::SettingId, setting_id_hash_ },
            { SettingsPayloadType::Value, ToEventData(value) }
        }
    });
}

void SettingWidgetBase::RequestSettingState() const {
    static constexpr auto caller = GetCallerName();

    if(!HasSetting())
        return;

    SettingsEventsChannel::GetInstance().PublishAsync({
        .source_id = caller.hash,
        .type = SettingsEventType::StateRequested,
        .payload = { { SettingsPayloadType::SettingId, setting_id_hash_ } }
    });
}

} // namespace eerie_leap::views::widgets

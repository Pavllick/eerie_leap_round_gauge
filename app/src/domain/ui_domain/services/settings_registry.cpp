#include <cerrno>
#include <utility>

#include <zephyr/logging/log.h>

#include "subsys/threading/scoped_mutex.h"
#include "utilities/string/string_helpers.h"

#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "settings_registry.h"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::domain::ui_domain::event_bus;

using eerie_leap::subsys::threading::ScopedMutex;
using eerie_leap::utilities::string::StringHelpers;

LOG_MODULE_REGISTER(settings_registry_logger);

SettingsRegistry::SettingsRegistry() {
    k_mutex_init(&lock_);
}

int SettingsRegistry::Register(std::string setting_id, Binding binding) {
    if(setting_id.empty()) {
        LOG_ERR("A setting cannot be registered without an id.");
        return -EINVAL;
    }

    ScopedMutex guard(lock_);

    auto [it, inserted] = bindings_.emplace(std::move(setting_id), std::move(binding));
    if(!inserted) {
        LOG_ERR("Setting '%s' is already registered.", it->first.c_str());
        return -EEXIST;
    }

    return 0;
}

int SettingsRegistry::Unregister(std::string_view setting_id) {
    ScopedMutex guard(lock_);

    auto it = bindings_.find(setting_id);
    if(it == bindings_.end())
        return -ENOENT;

    bindings_.erase(it);

    return 0;
}

std::optional<SettingsRegistry::Binding> SettingsRegistry::Find(std::string_view setting_id) const {
    ScopedMutex guard(lock_);

    auto it = bindings_.find(setting_id);

    return it == bindings_.end() ? std::nullopt : std::optional<Binding>(it->second);
}

std::optional<ConfigValue> SettingsRegistry::Get(std::string_view setting_id) const {
    auto binding = Find(setting_id);
    if(!binding.has_value() || !binding->get)
        return std::nullopt;

    return binding->get();
}

int SettingsRegistry::Set(std::string_view setting_id, const ConfigValue& value) {
    auto binding = Find(setting_id);
    if(!binding.has_value())
        return -ENOENT;

    if(!binding->set)
        return -ENOTSUP;

    // Read back around the write so a value the binding clamped, quantized or
    // did not move at all does not wake every subscriber for nothing.
    std::optional<ConfigValue> previous;
    if(binding->get)
        previous = binding->get();

    int res = binding->set(value);
    if(res != 0)
        return res;

    if(previous.has_value() && binding->get() == *previous)
        return 0;

    UiEventPayload payload;
    payload[UiPayloadType::SettingId] = StringHelpers::GetHash(setting_id);

    UiEventBus::GetInstance().PublishAsync({
        .type = UiEventType::SettingChanged,
        .payload = payload
    });

    return 0;
}

int SettingsRegistry::Commit(std::string_view setting_id) {
    auto binding = Find(setting_id);
    if(!binding.has_value())
        return -ENOENT;

    if(!binding->commit)
        return -ENOTSUP;

    return binding->commit();
}

std::optional<SettingRange> SettingsRegistry::GetRange(std::string_view setting_id) const {
    auto binding = Find(setting_id);
    if(!binding.has_value())
        return std::nullopt;

    return binding->range;
}

} // namespace eerie_leap::domain::ui_domain::services

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <zephyr/ztest.h>

#include "utilities/string/string_helpers.h"

#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "domain/settings_domain/utilities/settings_registry.h"

using eerie_leap::domain::ui_domain::event_bus::UiEvent;
using eerie_leap::domain::ui_domain::event_bus::UiEventBus;
using eerie_leap::domain::ui_domain::event_bus::UiEventType;
using eerie_leap::domain::ui_domain::event_bus::UiPayloadType;
using eerie_leap::domain::ui_domain::event_bus::UiSubscriptionHandle;
using eerie_leap::domain::settings_domain::utilities::SettingRange;
using eerie_leap::domain::settings_domain::utilities::SettingsRegistry;
using eerie_leap::domain::settings_domain::utilities::ToSettingBoolean;
using eerie_leap::domain::settings_domain::utilities::ToSettingNumber;
using eerie_leap::utilities::string::StringHelpers;
using eerie_leap::utilities::type::ConfigValue;

namespace {

constexpr int DISPATCH_TIMEOUT_MS = 1000;
constexpr int NO_DISPATCH_TIMEOUT_MS = 200;
constexpr const char* BRIGHTNESS_ID = "display.brightness";

// Stands in for the domain service a binding would normally drive.
struct FakeSetting {
    int value = 0;
    int commits = 0;
    int set_result = 0;
    int commit_result = 0;
};

SettingsRegistry::Binding MakeBinding(FakeSetting& setting) {
    return SettingsRegistry::Binding {
        .get = [&setting] { return ConfigValue { setting.value }; },
        .set = [&setting](const ConfigValue& value) {
            if(setting.set_result != 0)
                return setting.set_result;

            auto number = ToSettingNumber(value);
            if(!number.has_value())
                return -EINVAL;

            setting.value = static_cast<int>(*number);

            return 0;
        },
        .commit = [&setting] {
            if(setting.commit_result != 0)
                return setting.commit_result;

            setting.commits++;

            return 0;
        },
        .range = SettingRange { .min = 0, .max = 255, .step = 5 }
    };
}

// UiEventBus is a singleton, so a probe subscribes for the lifetime of one test
// only. ProcessEvent snapshots handlers before invoking them, so what the
// handler touches has to outlive the probe rather than the subscription.
class SettingProbe {
public:
    SettingProbe() : state_(std::make_shared<State>()) {
        k_sem_init(&state_->delivered, 0, K_SEM_MAX_LIMIT);

        auto subscription = UiEventBus::GetInstance().Subscribe(
            UiEventType::SettingChanged,
            [state = state_](const UiEvent& event) { Record(*state, event); });

        if(subscription)
            handle_.emplace(std::move(*subscription));
    }

    ~SettingProbe() {
        if(handle_.has_value())
            UiEventBus::GetInstance().Unsubscribe(handle_.value());
    }

    SettingProbe(const SettingProbe&) = delete;
    SettingProbe& operator=(const SettingProbe&) = delete;

    [[nodiscard]] bool IsSubscribed() const { return handle_.has_value(); }
    bool WaitForEvent() { return k_sem_take(&state_->delivered, K_MSEC(DISPATCH_TIMEOUT_MS)) == 0; }
    bool WaitForNoEvent() { return k_sem_take(&state_->delivered, K_MSEC(NO_DISPATCH_TIMEOUT_MS)) != 0; }

    [[nodiscard]] size_t Count() const { return state_->setting_ids.size(); }
    [[nodiscard]] const std::vector<uint32_t>& SettingIds() const { return state_->setting_ids; }

private:
    struct State {
        std::vector<uint32_t> setting_ids;
        k_sem delivered{};
    };

    static void Record(State& state, const UiEvent& event) {
        if(auto it = event.payload.find(UiPayloadType::SettingId); it != event.payload.end()) {
            if(auto* setting_id = std::get_if<uint32_t>(&it->second))
                state.setting_ids.push_back(*setting_id);
        }

        k_sem_give(&state.delivered);
    }

    std::shared_ptr<State> state_;
    std::optional<UiSubscriptionHandle> handle_;
};

int RegisteredValue(const SettingsRegistry& registry, const char* setting_id) {
    auto value = registry.Get(setting_id);
    zassert_true(value.has_value(), "Expected a value for '%s'.", setting_id);

    auto number = ToSettingNumber(*value);
    zassert_true(number.has_value(), "Expected a numeric value for '%s'.", setting_id);

    return static_cast<int>(*number);
}

// Let anything the previous test published drain while no probe is subscribed.
void SettleEventBus(void*) {
    k_msleep(20);
}

} // namespace

ZTEST_SUITE(settings_registry, NULL, NULL, SettleEventBus, NULL, NULL);

ZTEST(settings_registry, test_unregistered_setting_is_not_resolved) {
    SettingsRegistry registry;

    zassert_false(registry.Get(BRIGHTNESS_ID).has_value());
    zassert_false(registry.GetRange(BRIGHTNESS_ID).has_value());
    zassert_equal(registry.Set(BRIGHTNESS_ID, ConfigValue { 10 }), -ENOENT);
    zassert_equal(registry.Commit(BRIGHTNESS_ID), -ENOENT);
}

ZTEST(settings_registry, test_empty_setting_id_is_rejected) {
    SettingsRegistry registry;
    FakeSetting setting;

    zassert_equal(registry.Register("", MakeBinding(setting)), -EINVAL);
}

ZTEST(settings_registry, test_duplicate_registration_is_rejected) {
    SettingsRegistry registry;
    FakeSetting setting;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));
    zassert_equal(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)), -EEXIST);
}

// A binding captures the service that owns it, so withdrawing it has to be possible.
ZTEST(settings_registry, test_unregister_withdraws_the_binding) {
    SettingsRegistry registry;
    FakeSetting setting;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));
    zassert_ok(registry.Unregister(BRIGHTNESS_ID));

    zassert_false(registry.Get(BRIGHTNESS_ID).has_value());
    zassert_false(registry.GetRange(BRIGHTNESS_ID).has_value());
    zassert_equal(registry.Set(BRIGHTNESS_ID, ConfigValue { 10 }), -ENOENT);
    zassert_equal(registry.Unregister(BRIGHTNESS_ID), -ENOENT);

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));
}

ZTEST(settings_registry, test_range_is_returned) {
    SettingsRegistry registry;
    FakeSetting setting;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));

    auto range = registry.GetRange(BRIGHTNESS_ID);
    zassert_true(range.has_value());
    zassert_equal(range->min, 0);
    zassert_equal(range->max, 255);
    zassert_equal(range->step, 5);
}

ZTEST(settings_registry, test_set_applies_the_binding_and_publishes) {
    SettingsRegistry registry;
    FakeSetting setting;
    SettingProbe probe;

    zassert_true(probe.IsSubscribed());
    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));

    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 160 }));
    zassert_equal(setting.value, 160);
    zassert_equal(RegisteredValue(registry, BRIGHTNESS_ID), 160);

    zassert_true(probe.WaitForEvent());
    zassert_equal(probe.Count(), 1);
    zassert_equal(probe.SettingIds()[0], StringHelpers::GetHash(BRIGHTNESS_ID));
}

ZTEST(settings_registry, test_set_accepts_a_double) {
    SettingsRegistry registry;
    FakeSetting setting;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));
    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 42.0 }));

    zassert_equal(setting.value, 42);
}

// A failed Set() must not tell subscribed widgets that the value moved.
ZTEST(settings_registry, test_failed_set_is_not_published) {
    SettingsRegistry registry;
    FakeSetting setting;
    setting.set_result = -EIO;

    SettingProbe probe;
    zassert_true(probe.IsSubscribed());
    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));

    zassert_equal(registry.Set(BRIGHTNESS_ID, ConfigValue { 160 }), -EIO);
    zassert_true(probe.WaitForNoEvent());
    zassert_equal(probe.Count(), 0);
}

// A slider drag lands on the same quantized value repeatedly; republishing each
// one would wake every subscriber for nothing.
ZTEST(settings_registry, test_unchanged_value_is_not_published) {
    SettingsRegistry registry;
    FakeSetting setting;
    setting.value = 160;

    SettingProbe probe;
    zassert_true(probe.IsSubscribed());
    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));

    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 160 }));
    zassert_true(probe.WaitForNoEvent());
    zassert_equal(probe.Count(), 0);

    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 161 }));
    zassert_true(probe.WaitForEvent());
    zassert_equal(probe.Count(), 1);
}

ZTEST(settings_registry, test_commit_is_separate_from_set) {
    SettingsRegistry registry;
    FakeSetting setting;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));

    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 10 }));
    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 20 }));
    zassert_equal(setting.commits, 0);

    zassert_ok(registry.Commit(BRIGHTNESS_ID));
    zassert_equal(setting.commits, 1);
}

ZTEST(settings_registry, test_binding_errors_are_reported) {
    SettingsRegistry registry;
    FakeSetting setting;
    setting.commit_result = -EIO;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(setting)));
    zassert_equal(registry.Commit(BRIGHTNESS_ID), -EIO);
}

ZTEST(settings_registry, test_read_only_binding_rejects_writes) {
    SettingsRegistry registry;

    zassert_ok(registry.Register("device.serial", SettingsRegistry::Binding {
        .get = [] { return ConfigValue { 7 }; }
    }));

    zassert_equal(RegisteredValue(registry, "device.serial"), 7);
    zassert_equal(registry.Set("device.serial", ConfigValue { 8 }), -ENOTSUP);
    zassert_equal(registry.Commit("device.serial"), -ENOTSUP);
}

ZTEST(settings_registry, test_settings_are_isolated_by_id) {
    SettingsRegistry registry;
    FakeSetting brightness;
    FakeSetting timeout;

    zassert_ok(registry.Register(BRIGHTNESS_ID, MakeBinding(brightness)));
    zassert_ok(registry.Register("display.timeout", MakeBinding(timeout)));

    zassert_ok(registry.Set(BRIGHTNESS_ID, ConfigValue { 100 }));

    zassert_equal(brightness.value, 100);
    zassert_equal(timeout.value, 0);
}

ZTEST(settings_registry, test_value_conversion_helpers) {
    zassert_equal(ToSettingNumber(ConfigValue { 5 }).value_or(-1), 5.0);
    zassert_equal(ToSettingNumber(ConfigValue { 5.5 }).value_or(-1), 5.5);
    zassert_false(ToSettingNumber(ConfigValue { true }).has_value());

    zassert_true(ToSettingBoolean(ConfigValue { true }).value_or(false));
    zassert_false(ToSettingBoolean(ConfigValue { 1 }).has_value());
}

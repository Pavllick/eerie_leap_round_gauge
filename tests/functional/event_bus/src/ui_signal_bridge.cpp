#include <cstdint>
#include <optional>
#include <variant>

#include <zephyr/ztest.h>

#include "subsys/event_bus/event_bus.h"
#include "subsys/event_bus/event_channel.h"

#include "domain/logging_domain/event_bus/logging_events_channel.h"
#include "domain/ui_domain/event_bus/ui_signal_channel.h"

#include "event_bus/ui_signal_bridge.h"

using eerie_leap::domain::logging_domain::event_bus::LoggingEventsChannel;
using eerie_leap::domain::logging_domain::event_bus::LoggingEventType;
using eerie_leap::domain::logging_domain::event_bus::LoggingPayloadType;
using eerie_leap::domain::ui_domain::event_bus::UiSignalChannel;
using eerie_leap::domain::ui_domain::event_bus::UiSignalPayloadType;
using eerie_leap::domain::ui_domain::event_bus::UiSignalType;
using eerie_leap::event_bus::UiSignalBridge;
using eerie_leap::subsys::event_bus::AnySubscription;
using eerie_leap::subsys::event_bus::CreateScopedSubscription;
using eerie_leap::subsys::event_bus::EventBus;

namespace {

constexpr int BUS_STACK_SIZE = 4096;

// Both ends of the bridge are inert until a bus drains them, so the suite owns one.
class TestEventBus : public EventBus {
public:
    TestEventBus() : EventBus("signal_test_bus", BUS_STACK_SIZE) {
        RegisterChannel(LoggingEventsChannel::GetInstance());
        RegisterChannel(UiSignalChannel::GetInstance());
    }
};

// The channels are singletons, so a probe subscribes for the lifetime of one test only.
class SignalProbe {
public:
    explicit SignalProbe(UiSignalType signal) {
        subscription_ = CreateScopedSubscription(
            UiSignalChannel::GetInstance(),
            signal,
            [this](const UiSignalChannel::EventMessage& event) {
                ++calls_;

                if(auto it = event.payload.find(UiSignalPayloadType::Value); it != event.payload.end())
                    if(const auto* value = std::get_if<bool>(&it->second))
                        last_value_ = *value;
            });
    }

    [[nodiscard]] int Calls() const { return calls_; }
    [[nodiscard]] std::optional<bool> LastValue() const { return last_value_; }

private:
    AnySubscription subscription_;
    int calls_ = 0;
    std::optional<bool> last_value_;
};

void PublishLoggingStatus(bool is_active) {
    LoggingEventsChannel::GetInstance().Publish({
        .source_id = 0x7E57,
        .type = LoggingEventType::StatusUpdated,
        .payload = { { LoggingPayloadType::IsActive, is_active } }
    });
}

} // namespace

ZTEST_SUITE(ui_signal_bridge, NULL, NULL, NULL, NULL, NULL);

ZTEST(ui_signal_bridge, test_a_linked_signal_reaches_the_ui_signal_channel) {
    TestEventBus bus;
    UiSignalBridge bridge;
    bridge.Initialize();

    SignalProbe probe(UiSignalType::LoggingActive);

    PublishLoggingStatus(true);

    zassert_equal(probe.Calls(), 1);
    zassert_true(probe.LastValue().has_value(), "the payload value was dropped in the bridge");
    zassert_true(probe.LastValue().value());
}

ZTEST(ui_signal_bridge, test_the_payload_value_survives_the_republish) {
    TestEventBus bus;
    UiSignalBridge bridge;
    bridge.Initialize();

    SignalProbe probe(UiSignalType::LoggingActive);

    PublishLoggingStatus(false);

    zassert_equal(probe.Calls(), 1);
    zassert_true(probe.LastValue().has_value());
    zassert_false(probe.LastValue().value(), "the bridge must forward the value, not a placeholder");
}

ZTEST(ui_signal_bridge, test_an_unlinked_signal_never_fires) {
    TestEventBus bus;
    UiSignalBridge bridge;
    bridge.Initialize();

    SignalProbe unlinked(UiSignalType::None);

    PublishLoggingStatus(true);

    zassert_equal(unlinked.Calls(), 0, "only linked signals may be published");
}

ZTEST(ui_signal_bridge, test_a_source_event_without_the_linked_key_is_ignored) {
    TestEventBus bus;
    UiSignalBridge bridge;
    bridge.Initialize();

    SignalProbe probe(UiSignalType::LoggingActive);

    LoggingEventsChannel::GetInstance().Publish({
        .source_id = 0x7E57,
        .type = LoggingEventType::StatusUpdated,
        .payload = {}
    });

    zassert_equal(probe.Calls(), 0);
}

ZTEST(ui_signal_bridge, test_nothing_is_republished_once_the_bridge_is_gone) {
    TestEventBus bus;
    SignalProbe probe(UiSignalType::LoggingActive);

    {
        UiSignalBridge bridge;
        bridge.Initialize();

        PublishLoggingStatus(true);

        zassert_equal(probe.Calls(), 1);
    }

    PublishLoggingStatus(true);

    zassert_equal(probe.Calls(), 1, "the bridge must drop its links when it is destroyed");
}

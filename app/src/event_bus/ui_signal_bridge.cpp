#include "domain/logging_domain/event_bus/logging_events_channel.h"

#include "ui_signal_bridge.h"

namespace eerie_leap::event_bus {

using namespace eerie_leap::domain::logging_domain::event_bus;

void UiSignalBridge::Initialize() {
    Link(LoggingEventsChannel::GetInstance(), LoggingEventType::StatusUpdated, LoggingPayloadType::IsActive, UiSignalType::LoggingActive);
}

} // namespace eerie_leap::event_bus

#include <zephyr/logging/log.h>

#include "subsys/time/time_helpers.hpp"
#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "reading_processor.h"

namespace eerie_leap::domain::canbus_domain::processors {

using namespace eerie_leap::subsys::time;
using namespace eerie_leap::domain::ui_domain::event_bus;

LOG_MODULE_REGISTER(reading_processor_logger);

ReadingProcessor::ReadingProcessor() : service_running_(ATOMIC_INIT(0)) { }

void ReadingProcessor::SubmitToEventBus(const SensorReadingDto& reading) {
    if(reading.status != ReadingStatus::PROCESSED) {
        printf("Reading status not PROCESSED for sensor: %lu", reading.sensor_id_hash);
        return;
    }

    UiEventPayload payload;
    payload[UiPayloadType::SensorId] = reading.sensor_id_hash;
    payload[UiPayloadType::Value] = reading.value;

    UiEventBus::GetInstance().PublishAsync({
        .type = UiEventType::SensorDataUpdated,
        .payload = payload
    });
}

int ReadingProcessor::Process(const SensorReadingDto& reading) {
    SubmitToEventBus(reading);

    return 0;
}

} // namespace eerie_leap::domain::canbus_domain::processors

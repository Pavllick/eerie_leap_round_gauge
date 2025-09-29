#include <zephyr/logging/log.h>

#include "utilities/time/time_helpers.hpp"

#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "reading_processor.h"

namespace eerie_leap::domain::interface_domain::processors {

using namespace eerie_leap::utilities::time;
using namespace eerie_leap::domain::ui_domain::event_bus;

LOG_MODULE_REGISTER(reading_processor_logger);

ReadingProcessor::ReadingProcessor() : service_running_(ATOMIC_INIT(0)) { }

void ReadingProcessor::Start() {
    atomic_set(&service_running_, 1);
    LOG_INF("Reading processor service started.");
}

void ReadingProcessor::Stop() {
    atomic_set(&service_running_, 0);
    LOG_INF("Reading processor service stopped.");
}

void ReadingProcessor::SubmitToEventBus(const SensorReadingDto& reading) {
    if(reading.status != ReadingStatus::PROCESSED) {
        LOG_DBG("Reading status not PROCESSED for sensor: %lu", reading.sensor_id_hash);
        return;
    }

    UiEventPayload payload;
    payload[UiPayloadType::SENSOR_ID] = reading.sensor_id_hash;
    payload[UiPayloadType::VALUE] = reading.value;

    UiEvent event {
        .type = UiEventType::SENSOR_DATA_UPDATED,
        .payload = payload
    };

    UiEventBus::GetInstance().PublishAsync(event);
}

int ReadingProcessor::Process(SensorReadingDto reading) {
    if(atomic_get(&service_running_) == 0)
        return -1;

    SubmitToEventBus(reading);

    return 0;
}

} // namespace eerie_leap::domain::interface_domain::services

#pragma once

#include <zephyr/sys/atomic.h>

#include "domain/sensor_domain/models/sensor_reading_dto.h"
#include "i_interface_processor.h"

namespace eerie_leap::domain::interface_domain::processors {

using namespace eerie_leap::domain::sensor_domain::models;

class ReadingProcessor : public IInterfaceProcessor<SensorReadingDto> {
private:
    atomic_t service_running_;

    static void SubmitToEventBus(const SensorReadingDto& reading);

public:
    ReadingProcessor();

    int Process(const SensorReadingDto& reading) override;
};

} // namespace eerie_leap::domain::interface_domain::services

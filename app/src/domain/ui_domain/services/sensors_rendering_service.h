#pragma once

#include <memory>
#include <vector>

#include <zephyr/kernel.h>

#include "subsys/threading/work_queue_thread.h"
#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"
#include "domain/sensor_domain/readers/canbus_sensors_reader.h"

#include "sensors_rendering_task.hpp"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::subsys::threading;
using namespace eerie_leap::domain::sensor_domain::utilities;
using namespace eerie_leap::domain::sensor_domain::readers;

class SensorsRenderingService {
private:
    static constexpr int thread_stack_size_ = 4096;
    static constexpr int thread_priority_ = 6;
    std::unique_ptr<WorkQueueThread> work_queue_thread_;

    static constexpr uint32_t SENSORS_REFRESH_RATE_MS = 30;

    // std::shared_ptr<CanbusConfigurationManager> canbus_configuration_manager_;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame_;
    std::shared_ptr<CanbusSensorsReader> canbus_sensors_reader_;

    std::optional<WorkQueueTask<SensorsRenderingTask>> work_queue_task_;

    static WorkQueueTaskResult ProcessWorkTask(SensorsRenderingTask* task);
    static void SubmitToEventBus(const SensorReadingDto& reading);

public:
    SensorsRenderingService(
        std::shared_ptr<SensorReadingsFrame> sensor_readings_frame,
        std::shared_ptr<CanbusSensorsReader> canbus_sensors_reader);
    ~SensorsRenderingService() = default;

    void Initialize();

    void Start();
    void Restart();
    void Pause();
    void Resume();
};

} // namespace eerie_leap::domain::ui_domain::services

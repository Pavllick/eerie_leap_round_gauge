#pragma once

#include <memory>
#include <vector>

#include <zephyr/kernel.h>

#include "subsys/threading/work_queue_thread.h"
#include "subsys/time/time_service.h"
#include "domain/canbus_domain/configuration/canbus_configuration_manager.h"
#include "domain/canbus_domain/services/canbus_service.h"
#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"

#include "sensors_rendering_task.hpp"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::subsys::threading;
using namespace eerie_leap::subsys::time;
using namespace eerie_leap::domain::canbus_domain::configuration;
using namespace eerie_leap::domain::canbus_domain::services;
using namespace eerie_leap::domain::sensor_domain::utilities;

class SensorsRenderingService {
private:
    static constexpr int thread_stack_size_ = 4096;
    static constexpr int thread_priority_ = 6;
    std::unique_ptr<WorkQueueThread> work_queue_thread_;
    std::optional<WorkQueueTask<SensorsRenderingTask>> work_queue_task_;

    static constexpr uint32_t SENSORS_REFRESH_RATE_MS = 30;

    std::shared_ptr<TimeService> time_service_;
    std::shared_ptr<CanbusConfigurationManager> canbus_configuration_manager_;
    std::shared_ptr<CanbusService> canbus_service_;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame_;

    static WorkQueueTaskResult ProcessWorkTask(SensorsRenderingTask* task);
    static void SubmitToEventBus(const SensorReading& reading);

public:
    SensorsRenderingService(
        std::shared_ptr<TimeService> time_service,
        std::shared_ptr<CanbusConfigurationManager> canbus_configuration_manager,
        std::shared_ptr<CanbusService> canbus_service,
        std::shared_ptr<SensorReadingsFrame> sensor_readings_frame);
    ~SensorsRenderingService() = default;

    void Initialize();

    void Start();
    void Restart();
    void Pause();
    void Resume();
};

} // namespace eerie_leap::domain::ui_domain::services

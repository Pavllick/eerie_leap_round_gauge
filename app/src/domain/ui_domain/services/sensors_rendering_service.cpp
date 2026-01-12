#include <span>

#include "subsys/time/time_helpers.hpp"
#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "sensors_rendering_service.h"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::subsys::time;
using namespace eerie_leap::domain::sensor_domain::models;
using namespace eerie_leap::domain::ui_domain::event_bus;

LOG_MODULE_REGISTER(processing_scheduler_logger);

SensorsRenderingService::SensorsRenderingService(
    std::shared_ptr<TimeService> time_service,
    std::shared_ptr<CanbusConfigurationManager> canbus_configuration_manager,
    std::shared_ptr<CanbusService> canbus_service,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame)
        : work_queue_thread_(nullptr),
        time_service_(std::move(time_service)),
        canbus_configuration_manager_(std::move(canbus_configuration_manager)),
        canbus_service_(std::move(canbus_service)),
        sensor_readings_frame_(std::move(sensor_readings_frame)) {};

void SensorsRenderingService::Initialize() {
    work_queue_thread_ = std::make_unique<WorkQueueThread>(
        "sensors_rendering_service",
        thread_stack_size_,
        thread_priority_);
    work_queue_thread_->Initialize();

    auto task = std::make_unique<SensorsRenderingTask>();
    task->refresh_rate_ms = K_MSEC(SENSORS_REFRESH_RATE_MS);
    task->sensor_readings_frame = sensor_readings_frame_;

    work_queue_task_ = work_queue_thread_->CreateTask(ProcessWorkTask, std::move(task));
}

WorkQueueTaskResult SensorsRenderingService::ProcessWorkTask(SensorsRenderingTask* task) {
    try {
        for(const auto& [_, reading] : task->sensor_readings_frame->GetReadings())
            SubmitToEventBus(*reading);

        task->sensor_readings_frame->ClearReadings();
    } catch (const std::exception& e) {
        LOG_DBG("Failed to render sensors. Error: %s", e.what());
    }

    return {
        .reschedule = true,
        .delay = task->refresh_rate_ms
    };
}

void SensorsRenderingService::SubmitToEventBus(const SensorReading& reading) {
    UiEventPayload payload;
    payload[UiPayloadType::SensorId] = reading.sensor->id_hash;
    payload[UiPayloadType::Value] = reading.value.value_or(0.0f);

    UiEventBus::GetInstance().PublishAsync({
        .type = UiEventType::SensorDataUpdated,
        .payload = payload
    });
}

void SensorsRenderingService::Start() {
    work_queue_task_.value().Schedule();

    LOG_INF("Processing Scheduler Service started.");
}

void SensorsRenderingService::Restart() {
    Pause();
    sensor_readings_frame_->ClearReadings();
    Start();
}

void SensorsRenderingService::Pause() {
    while(work_queue_task_.value().Cancel())
        k_sleep(K_MSEC(1));

    LOG_INF("Processing Scheduler Service stopped.");
}

void SensorsRenderingService::Resume() {
    Start();
}

} // namespace eerie_leap::domain::ui_domain::services

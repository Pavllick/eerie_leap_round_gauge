#include <zephyr/logging/log.h>

#include "utilities/memory/heap_allocator.h"
#include "reading_processing_service.h"
#include "reading_handler.h"

namespace eerie_leap::domain::sensor_domain::services {

using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(reading_processing_service_logger);

K_KERNEL_STACK_MEMBER(ReadingProcessingService::stack_area_, ReadingProcessingService::k_stack_size_);

ReadingProcessingService::ReadingProcessingService() {
    k_sem_init(&processing_semaphore_, 1, 1);
};

void ReadingProcessingService::Initialize() {
    k_work_queue_init(&work_q);
    k_work_queue_start(&work_q, stack_area_,
        K_THREAD_STACK_SIZEOF(stack_area_),
        k_priority_, NULL);

    LOG_INF("Reading processing service initialized.");
}

void ReadingProcessingService::ProcessReadingWorkTask(k_work* work) {
    ReadingTask* task = CONTAINER_OF(work, ReadingTask, work);

    if(k_sem_take(task->processing_semaphore, PROCESSING_TIMEOUT) != 0) {
        LOG_ERR("Lock take timed out for sensor: %lu", task->reading.sensor_id_hash);

        return;
    }

    LOG_INF("Sensor Reading - ID: %lu, Value: %.3f\n",
        task->reading.sensor_id_hash,
        task->reading.value);

    task->reading_handler(task->reading);

    k_sem_give(task->processing_semaphore);
}

int ReadingProcessingService::RegisterReadingHandler(size_t sensor_id_hash, ReadingHandler handler) {
    auto task = make_shared_ext<ReadingTask>();
    task->processing_semaphore = &processing_semaphore_;
    task->reading_handler = std::move(handler);

    memset(&task->reading, 0, sizeof(SensorReadingDto));

    k_work_init(&task->work, ProcessReadingWorkTask);
    reading_tasks_[sensor_id_hash] = std::move(task);

    return 0;
}

int ReadingProcessingService::ProcessReading(SensorReadingDto reading) {
    auto task = reading_tasks_.find(reading.sensor_id_hash);
    if(task == reading_tasks_.end())
        return -1;

    task->second->reading = reading;

    if(!k_work_is_pending(&task->second->work))
        k_work_submit_to_queue(&work_q, &task->second->work);

    return 0;
}

} // namespace eerie_leap::domain::sensor_domain::services

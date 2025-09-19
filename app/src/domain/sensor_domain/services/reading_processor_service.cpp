#include <zephyr/logging/log.h>

#include "utilities/memory/heap_allocator.h"
#include "utilities/time/time_helpers.hpp"
#include "reading_processor_service.h"

namespace eerie_leap::domain::sensor_domain::services {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::time;

LOG_MODULE_REGISTER(reading_processor_service_logger);

#ifdef CONFIG_SHARED_MULTI_HEAP
Z_KERNEL_STACK_DEFINE_IN(ReadingProcessorService::stack_area_, ReadingProcessorService::k_stack_size_, __attribute__((section(".ext_ram.bss"))));
#else
K_KERNEL_STACK_MEMBER(ReadingProcessorService::stack_area_, ReadingProcessorService::k_stack_size_);
#endif

ReadingProcessorService::ReadingProcessorService() {
    k_sem_init(&processing_semaphore_, 1, 1);
};

void ReadingProcessorService::Initialize() {
    k_work_queue_init(&work_q);
    k_work_queue_start(&work_q, stack_area_,
        K_THREAD_STACK_SIZEOF(stack_area_),
        k_priority_, NULL);

    k_thread_name_set(&work_q.thread, "reading_processor");

    LOG_INF("Reading processor service initialized.");
}

void ReadingProcessorService::ProcessReadingWorkTask(k_work* work) {
    ReadingTask* task = CONTAINER_OF(work, ReadingTask, work);

    if(k_sem_take(task->processing_semaphore, PROCESSING_TIMEOUT) != 0) {
        LOG_ERR("Lock take timed out for sensor: %lu", task->reading.sensor_id_hash);

        return;
    }

    LOG_DBG("Sensor Reading - ID: %lu, Guid: %llu, Value: %.3f, Time: %s\n",
        task->reading.sensor_id_hash,
        task->reading.id.AsUint64(),
        task->reading.value,
        TimeHelpers::GetFormattedString(TimeHelpers::FromMilliseconds(task->reading.timestamp_ms)).c_str());

    for(auto& handler : task->reading_handlers)
        handler(task->reading);

    k_sem_give(task->processing_semaphore);
}

int ReadingProcessorService::RegisterReadingHandler(uint32_t sensor_id_hash, ReadingHandler handler) {
    if(!sensors_reading_tasks_.contains(sensor_id_hash)) {
        auto task = make_shared_ext<ReadingTask>();
        task->processing_semaphore = &processing_semaphore_;

        memset(&task->reading, 0, sizeof(SensorReadingDto));

        k_work_init(&task->work, ProcessReadingWorkTask);
        sensors_reading_tasks_[sensor_id_hash] = std::move(task);
    }

    sensors_reading_tasks_[sensor_id_hash]->reading_handlers.push_back(std::move(handler));

    return 0;
}

int ReadingProcessorService::ProcessReading(SensorReadingDto reading) {
    auto task = sensors_reading_tasks_.find(reading.sensor_id_hash);
    if(task == sensors_reading_tasks_.end())
        return -1;

    task->second->reading = reading;

    if(!k_work_is_pending(&task->second->work))
        k_work_submit_to_queue(&work_q, &task->second->work);

    return 0;
}

} // namespace eerie_leap::domain::sensor_domain::services

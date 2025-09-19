#pragma once

#include <memory>
#include <unordered_map>

#include <zephyr/kernel.h>

#include "domain/interface_domain/types/sensor_reading_dto.h"
#include "reading_task.hpp"
#include "reading_handler.h"

namespace eerie_leap::domain::sensor_domain::services {

class ReadingProcessorService {
private:
    static constexpr int k_stack_size_ = CONFIG_EERIE_LEAP_READING_PROCESSOR_THREAD_STACK_SIZE;
    static constexpr int k_priority_ = K_PRIO_COOP(10);

    static z_thread_stack_element stack_area_[k_stack_size_];
    k_work_q work_q;

    k_sem processing_semaphore_;
    static constexpr k_timeout_t PROCESSING_TIMEOUT = K_MSEC(200);

    std::unordered_map<uint32_t, std::shared_ptr<ReadingTask>> sensors_reading_tasks_;

    static void ProcessReadingWorkTask(k_work* work);

public:
    ReadingProcessorService();

    void Initialize();

    int RegisterReadingHandler(uint32_t sensor_id_hash, ReadingHandler handler);
    int ProcessReading(SensorReadingDto reading);
};

} // namespace eerie_leap::domain::sensor_domain::services

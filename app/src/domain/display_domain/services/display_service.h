#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include <zephyr/kernel.h>

#include "subsys/threading/work_queue_task.h"
#include "subsys/threading/work_queue_task_result.h"
#include "subsys/threading/work_queue_thread.h"

#include "domain/display_domain/configuration/display_configuration_manager.h"
#include "domain/display_domain/models/display_configuration.h"

namespace eerie_leap::domain::display_domain::services {

using eerie_leap::subsys::threading::WorkQueueTask;
using eerie_leap::subsys::threading::WorkQueueTaskResult;
using eerie_leap::subsys::threading::WorkQueueThread;
using eerie_leap::domain::display_domain::configuration::DisplayConfigurationManager;
using eerie_leap::domain::display_domain::models::DisplayConfiguration;

// Owns the panel state (brightness, blanking) and its persistence. The only
// place in the app that talks to the display driver outside of LVGL's own
// flush path.
class DisplayService {
private:
    struct PersistTask {
        DisplayService* service = nullptr;
    };

    static constexpr k_timeout_t persist_debounce_ = K_MSEC(1000);

    std::shared_ptr<DisplayConfigurationManager> configuration_manager_;
    std::shared_ptr<WorkQueueThread> config_work_queue_thread_;

    DisplayConfiguration configuration_;
    mutable k_mutex lock_;

    std::optional<WorkQueueTask<PersistTask>> persist_task_;

    bool is_initialized_ = false;

    int ApplyBrightness(uint8_t value);
    int ApplyBlanking(bool enabled);

    static WorkQueueTaskResult PersistWork(PersistTask* task);
    void WriteConfiguration();

public:
    DisplayService(
        std::shared_ptr<DisplayConfigurationManager> configuration_manager,
        std::shared_ptr<WorkQueueThread> config_work_queue_thread);

    ~DisplayService();

    DisplayService(const DisplayService&) = delete;
    DisplayService& operator=(const DisplayService&) = delete;

    int Initialize();
    bool IsInitialized() const;

    int SetBrightness(uint8_t value);
    uint8_t GetBrightness() const;

    int SetBlanking(bool enabled);
    bool IsBlankingEnabled() const;

    // Re-reads the stored configuration and pushes it to the panel; used when a
    // configuration arrives from outside (BLE) rather than from a control.
    int Reload();

    // Hands the flash write to the configuration work queue: this is reached
    // from the LVGL renderer thread, which must not block. Repeated calls
    // coalesce into a single write.
    int Persist();
};

} // namespace eerie_leap::domain::display_domain::services

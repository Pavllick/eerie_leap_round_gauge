#include <cerrno>
#include <utility>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

#include "subsys/device_tree/dt_display.h"
#include "subsys/threading/scoped_mutex.h"

#include "domain/ui_domain/lvgl_lock.h"

#include "display_service.h"

namespace eerie_leap::domain::display_domain::services {

using eerie_leap::subsys::device_tree::DtDisplay;
using eerie_leap::subsys::threading::ScopedMutex;
using eerie_leap::domain::ui_domain::ScopedLvglLock;

LOG_MODULE_REGISTER(display_service_logger);

DisplayService::DisplayService(
    std::shared_ptr<DisplayConfigurationManager> configuration_manager,
    std::shared_ptr<WorkQueueThread> config_work_queue_thread)
    : configuration_manager_(std::move(configuration_manager)),
      config_work_queue_thread_(std::move(config_work_queue_thread)) {

    k_mutex_init(&lock_);
}

DisplayService::~DisplayService() {
    if(persist_task_.has_value())
        persist_task_->Cancel();
}

int DisplayService::Initialize() {
    const auto* display = DtDisplay::Get();
    if(display == nullptr || !device_is_ready(display)) {
        LOG_ERR("Display not ready, aborting.");
        return -ENODEV;
    }

    DisplayConfiguration snapshot;
    auto configuration = configuration_manager_->Get();
    if(configuration != nullptr)
        snapshot = *configuration;
    else
        LOG_WRN("No stored Display configuration, using defaults.");

    is_initialized_ = true;

    {
        ScopedMutex guard(lock_);
        configuration_ = snapshot;
    }

    if(config_work_queue_thread_ != nullptr) {
        auto task = std::make_unique<PersistTask>();
        task->service = this;

        persist_task_ = config_work_queue_thread_->CreateTask(PersistWork, std::move(task));
    }

    ApplyBlanking(snapshot.blanking_enabled);
    ApplyBrightness(snapshot.brightness);

    LOG_INF("Display service initialized. Brightness: %u.", snapshot.brightness);

    return 0;
}

bool DisplayService::IsInitialized() const {
    return is_initialized_;
}

int DisplayService::ApplyBrightness(uint8_t value) {
    // Initialize() is what proves the device exists; without it the driver call
    // below would dereference a null device.
    if(!is_initialized_)
        return -ENODEV;

    // LVGL's flush callback reaches the same panel from the renderer thread
    // while it holds this lock, so every driver call here has to take it too.
    // Zephyr's k_mutex is recursive, so a control widget calling in from inside
    // lv_timer_handler() re-locks harmlessly.
    ScopedLvglLock lvgl_guard;

    int res = display_set_brightness(DtDisplay::Get(), value);

    // A panel without brightness control must not make the setting fail, or the
    // control bound to it would be unusable on that board (native_sim included).
    if(res == -ENOTSUP || res == -ENOSYS) {
        LOG_WRN("Display does not support brightness control.");

        return 0;
    }

    return res;
}

int DisplayService::ApplyBlanking(bool enabled) {
    if(!is_initialized_)
        return -ENODEV;

    ScopedLvglLock lvgl_guard;

    int res = enabled
        ? display_blanking_on(DtDisplay::Get())
        : display_blanking_off(DtDisplay::Get());

    if(res == -ENOTSUP || res == -ENOSYS)
        return 0;

    return res;
}

int DisplayService::SetBrightness(uint8_t value) {
    int res = ApplyBrightness(value);
    if(res != 0) {
        LOG_ERR("Failed to set display brightness to %u. Error: %d.", value, res);
        return res;
    }

    ScopedMutex guard(lock_);
    configuration_.brightness = value;

    return 0;
}

uint8_t DisplayService::GetBrightness() const {
    ScopedMutex guard(lock_);

    return configuration_.brightness;
}

int DisplayService::SetBlanking(bool enabled) {
    int res = ApplyBlanking(enabled);
    if(res != 0) {
        LOG_ERR("Failed to set display blanking. Error: %d.", res);
        return res;
    }

    ScopedMutex guard(lock_);
    configuration_.blanking_enabled = enabled;

    return 0;
}

bool DisplayService::IsBlankingEnabled() const {
    ScopedMutex guard(lock_);

    return configuration_.blanking_enabled;
}

int DisplayService::Reload() {
    auto configuration = configuration_manager_->Get();
    if(configuration == nullptr)
        return -ENOENT;

    DisplayConfiguration snapshot = *configuration;

    ApplyBlanking(snapshot.blanking_enabled);
    ApplyBrightness(snapshot.brightness);

    ScopedMutex guard(lock_);
    configuration_ = snapshot;

    return 0;
}

int DisplayService::Persist() {
    if(!persist_task_.has_value())
        return -ENODEV;

    // Every commit pushes the deadline out, so a burst of them reaches flash once.
    persist_task_->Reschedule(persist_debounce_);

    return 0;
}

WorkQueueTaskResult DisplayService::PersistWork(PersistTask* task) {
    task->service->WriteConfiguration();

    return {
        .reschedule = false
    };
}

void DisplayService::WriteConfiguration() {
    DisplayConfiguration snapshot;
    {
        ScopedMutex guard(lock_);
        snapshot = configuration_;
    }

    if(!configuration_manager_->Update(snapshot))
        LOG_ERR("Failed to persist the Display configuration.");
}

} // namespace eerie_leap::domain::display_domain::services

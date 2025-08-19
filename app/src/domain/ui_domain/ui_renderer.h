#pragma once

#include <atomic>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <lvgl.h>


namespace eerie_leap::domain::ui_domain {

class UiRenderer {
private:
    static const device* display_dev_;

    static constexpr int k_stack_size_ = 4096;
    static constexpr int k_priority_ = K_PRIO_COOP(7);

    std::atomic<bool> running_ = false;
    static z_thread_stack_element stack_area_[k_stack_size_];
    k_tid_t thread_id_;
    k_thread thread_data_;

    void UiRendererThreadEntry();

    void Render();
    static void DisplayInvalidateCb(lv_event_t* e);

public:
    UiRenderer() {}
    int Initialize();

    k_tid_t Start();
    void Stop();
};

} // namespace eerie_leap::domain::ui_domain

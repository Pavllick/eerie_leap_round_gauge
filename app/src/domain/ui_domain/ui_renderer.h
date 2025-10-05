#pragma once

#include <atomic>

#include <zephyr/kernel.h>
#include <lvgl.h>


namespace eerie_leap::domain::ui_domain {

class UiRenderer {
private:
    static constexpr int k_stack_size_ = CONFIG_EERIE_LEAP_UI_RENDERER_THREAD_STACK_SIZE;
    static constexpr int k_priority_ = K_PRIO_COOP(7);

    static z_thread_stack_element stack_area_[k_stack_size_];
    k_tid_t thread_id_;
    k_thread thread_data_;

    std::atomic<bool> running_ = false;

    void UiRendererThreadEntry();

    void Render();
    static void DisplayInvalidateCb(lv_event_t* e);

public:
    UiRenderer() = default;
    int Initialize();

    k_tid_t Start();
    void Stop();
};

} // namespace eerie_leap::domain::ui_domain

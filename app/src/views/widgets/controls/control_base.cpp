#include <exception>
#include <utility>

#include <zephyr/logging/log.h>

#include "control_base.h"

namespace eerie_leap::views::widgets::controls {

LOG_MODULE_REGISTER(control_base_logger);

ControlBase::ControlBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context, bool consumes_gestures)
    : SettingWidgetBase(id, std::move(parent), std::move(context)) {

    // Without this a drag over an interactive control would climb to the gesture
    // root and navigate away instead of moving the control.
    if(consumes_gestures)
        lv_obj_remove_flag(container_->GetObject(), LV_OBJ_FLAG_GESTURE_BUBBLE);
}

ControlBase::~ControlBase() {
    DetachDispatch();

    // Removes every code registered for this (callback, instance) pair.
    if(event_object_ != nullptr)
        lv_obj_remove_event_cb_with_user_data(event_object_, EventCb, this);
}

void ControlBase::AttachEvents(lv_obj_t* object, std::initializer_list<lv_event_code_t> codes) {
    if(object == nullptr)
        return;

    // A stale registration would keep handing `this` to LVGL after destruction.
    if(event_object_ != nullptr && event_object_ != object)
        lv_obj_remove_event_cb_with_user_data(event_object_, EventCb, this);

    event_object_ = object;

    for(auto code : codes)
        lv_obj_add_event_cb(object, EventCb, code, this);
}

// Invoked by LVGL on the renderer thread, which already holds the LVGL lock.
// An exception escaping back into LVGL's C code would skip that unlock.
void ControlBase::EventCb(lv_event_t* e) {
    auto* control = static_cast<ControlBase*>(lv_event_get_user_data(e));
    if(control == nullptr)
        return;

    try {
        control->OnControlEvent(lv_event_get_code(e));
    } catch(const std::exception& ex) {
        LOG_ERR("Failed to handle control event. %s", ex.what());
    } catch(...) {
        LOG_ERR("Failed to handle control event.");
    }
}

void ControlBase::OnControlEvent(lv_event_code_t) { }

} // namespace eerie_leap::views::widgets::controls

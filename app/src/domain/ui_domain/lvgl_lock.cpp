#include <zephyr/kernel.h>

#include "lvgl_lock.h"

namespace eerie_leap::domain::ui_domain {

LvglLock::LvglLock() {
    k_mutex_init(&lock_);
}

LvglLock& LvglLock::GetInstance() {
    static LvglLock instance;
    return instance;
}

void LvglLock::Lock() {
    k_mutex_lock(&lock_, K_FOREVER);
}

void LvglLock::Unlock() {
    k_mutex_unlock(&lock_);
}

} // namespace eerie_leap::domain::ui_domain

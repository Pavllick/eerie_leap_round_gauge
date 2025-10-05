#include <zephyr/logging/log.h>

#include "dt_display.h"

namespace eerie_leap::subsys::device_tree {

LOG_MODULE_REGISTER(dt_display_logger);

const device* DtDisplay::display_dev_ = nullptr;

void DtDisplay::Initialize() {
#if DT_HAS_CHOSEN(zephyr_display)
    display_dev_ = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    LOG_INF("Display initialized.");
#endif
}

} // namespace eerie_leap::subsys::device_tree

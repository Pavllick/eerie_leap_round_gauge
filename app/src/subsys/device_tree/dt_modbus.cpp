#include <zephyr/logging/log.h>

#include "dt_modbus.h"

namespace eerie_leap::subsys::device_tree {

LOG_MODULE_REGISTER(dt_modbus_logger);

char* DtModbus::iface_ = nullptr;

void DtModbus::Initialize() {
#if DT_HAS_ALIAS(modbus0)
    iface_ = DEVICE_DT_NAME(MODBUS_NODE);
    LOG_INF("Modbus initialized.");
#endif
}

} // namespace eerie_leap::subsys::device_tree

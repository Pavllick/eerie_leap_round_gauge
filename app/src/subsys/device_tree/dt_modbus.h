#pragma once

#include <optional>

#include <zephyr/devicetree.h>
#include <zephyr/device.h>

#if DT_HAS_ALIAS(modbus0)
#define MODBUS_NODE DT_ALIAS(modbus0)
#endif

namespace eerie_leap::subsys::device_tree {

class DtModbus {
private:
    static std::optional<char*> iface_;

    DtModbus() = default;

public:
    static void Initialize();

    static std::optional<char*>& Get() { return iface_; }
};

} // namespace eerie_leap::subsys::device_tree

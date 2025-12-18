#pragma once

#include <unordered_map>

#include <zephyr/devicetree.h>
#include <zephyr/device.h>

#if defined(CONFIG_CAN) && DT_HAS_ALIAS(can0)
#define CAN0_NODE DT_ALIAS(can0)
#endif

#if defined(CONFIG_CAN) && DT_HAS_ALIAS(can1)
#define CAN1_NODE DT_ALIAS(can1)
#endif

namespace eerie_leap::subsys::device_tree {

class DtCanbus {
private:
    static std::unordered_map<uint8_t, const device*> canbus_devs_;

    DtCanbus() = default;

public:
    static void Initialize();

    static const device* Get(uint8_t bus_channel);
};

} // namespace eerie_leap::subsys::device_tree

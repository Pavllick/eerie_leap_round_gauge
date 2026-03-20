#pragma once

#include <vector>
#include <memory>

#include <zephyr/bluetooth/bluetooth.h>

namespace eerie_leap::subsys::bluetooth::utilities {

struct AdBuffer {
    std::vector<bt_data> descriptors;
    std::vector<std::unique_ptr<uint8_t[]>> payloads;

    [[nodiscard]] size_t size() const {
        return descriptors.size();
    }

    [[nodiscard]] bool empty() const {
        return descriptors.empty();
    }

    [[nodiscard]] const bt_data* data() const {
        return descriptors.data();
    }
};

} // namespace eerie_leap::subsys::bluetooth::utilities

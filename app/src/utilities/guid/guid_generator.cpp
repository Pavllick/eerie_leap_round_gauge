#include <zephyr/kernel.h>

#include "guid_generator.h"

namespace eerie_leap::utilities::guid {

Guid GuidGenerator::Generate() {
    return Guid {
        .device_hash = device_hash_,
        .counter = counter_.fetch_add(1, std::memory_order_relaxed),
        .timestamp = k_uptime_get_32()
    };
}

} // namespace eerie_leap::utilities::guid

#pragma once

#include <cstdint>

namespace eerie_leap::utilities::guid {

struct Guid {
    uint16_t device_hash;
    uint16_t counter;
    uint32_t timestamp;

    uint64_t AsUint64() const {
        return *reinterpret_cast<const uint64_t*>(this);
    }
} __attribute__((packed, aligned(1))); // Ensure no padding

} // namespace eerie_leap::utilities::guid
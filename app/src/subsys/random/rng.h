#pragma once

#include <zephyr/random/random.h>

namespace eerie_leap::subsys::random {

class Rng {
public:
    static void Get(void* dst, size_t len, bool secure = false);
    static uint8_t Get8(bool secure = false);
    static uint16_t Get16(bool secure = false);
    static uint32_t Get32(bool secure = false);
    static uint64_t Get64(bool secure = false);
};

} // namespace eerie_leap::subsys::random

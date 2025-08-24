#pragma once

#include <zephyr/random/random.h>

#include "rng.h"

namespace eerie_leap::subsys::random {

void Rng::Get(void* dst, size_t len, bool secure) {
    if(!secure) {
        sys_rand_get(dst, len);
        return;
    }

#ifdef CONFIG_HARDWARE_DEVICE_CS_GENERATOR
    sys_csrand_get(dst, len);
#else
    sys_rand_get(dst, len);
#endif
}

uint8_t Rng::Get8(bool secure) {
    uint8_t ret;
    Get(&ret, sizeof(ret), secure);

    return ret;
}

uint16_t Rng::Get16(bool secure) {
    uint16_t ret;
    Get(&ret, sizeof(ret), secure);

    return ret;
}

uint32_t Rng::Get32(bool secure) {
    uint32_t ret;
    Get(&ret, sizeof(ret), secure);

    return ret;
}

uint64_t Rng::Get64(bool secure) {
    uint64_t ret;
    Get(&ret, sizeof(ret), secure);

    return ret;
}

} // namespace eerie_leap::subsys::random

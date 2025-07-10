#pragma once

#include <atomic>
#include <memory>
#include <zephyr/random/random.h>

#include "guid.hpp"
#include "utilities/time/i_time_service.h"

namespace eerie_leap::utilities::guid {

using namespace std::chrono;
using namespace eerie_leap::utilities::time;

class GuidGenerator {
private:
    const uint16_t device_hash_;
    std::atomic<uint16_t> counter_;

public:
    explicit GuidGenerator() : device_hash_(sys_rand32_get()), counter_(0) {}

    Guid Generate();
};

} // namespace eerie_leap::utilities::guid
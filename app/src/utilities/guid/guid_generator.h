#pragma once

#include <atomic>
#include <memory>

#include "subsys/random/rng.h"

#include "guid.hpp"

namespace eerie_leap::utilities::guid {

using namespace eerie_leap::subsys::random;

class GuidGenerator {
private:
    const uint16_t device_hash_;
    std::atomic<uint16_t> counter_;

public:
    explicit GuidGenerator() : device_hash_(Rng::Get16(true)), counter_(0) {}

    Guid Generate();
};

} // namespace eerie_leap::utilities::guid

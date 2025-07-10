#pragma once

#include <cstdint>

namespace eerie_leap::domain::system_domain {

struct SystemConfiguration {
    uint32_t hw_version;
    uint32_t sw_version;
};

} // namespace eerie_leap::domain::system_domain

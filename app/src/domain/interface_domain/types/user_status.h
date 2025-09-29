#pragma once

#include <cstdint>
#include <cstddef>

namespace eerie_leap::domain::interface_domain::types {

enum class ComUserStatus : uint16_t {
    ERROR = 0,
    OK = 1,
    NOT_CONFIGURED = 2,
    START_LOGGING = 3,
    STOP_LOGGING = 4,
};

struct UserStatus {
    ComUserStatus status;
    bool is_ok;
};

} // namespace eerie_leap::domain::interface_domain::types

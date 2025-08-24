#pragma once

#include <cstdint>

namespace eerie_leap::domain::interface_domain::types {

enum class RequestType : uint16_t {
    GET_RESOLVE_SERVER_ID_GUID = 1,

    SET_RESOLVE_SERVER_ID = 127,
    SET_RESOLVE_SERVER_ID_GUID = 128,
    SET_READING = 129,
};

} // namespace eerie_leap::domain::interface_domain::types

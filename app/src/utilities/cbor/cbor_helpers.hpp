#pragma once

#include <string>
#include <cstdint>
#include <zcbor_common.h>

namespace eerie_leap::utilities::cbor {

class CborHelpers {
public:
    static std::string ToStdString(const zcbor_string& zstr) {
        return std::string(reinterpret_cast<const char*>(zstr.value), zstr.len);
    }

    static zcbor_string ToZcborString(const std::string* str) {
        return {
            .value = reinterpret_cast<const uint8_t*>(str->data()),
            .len = static_cast<uint_fast32_t>(str->size())
        };
    }
};

} // namespace eerie_leap::utilities::cbor

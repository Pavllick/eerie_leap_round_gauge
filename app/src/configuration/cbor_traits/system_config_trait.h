#pragma once

#include "cbor_trait.h"
#include <configuration/system_config/system_config.h>
#include "configuration/system_config/system_config_cbor_encode.h"
#include "configuration/system_config/system_config_cbor_decode.h"

namespace eerie_leap::configuration::traits {

template <>
struct CborTrait<SystemConfig> {
    static constexpr auto Encode = cbor_encode_SystemConfig;
    static constexpr auto Decode = cbor_decode_SystemConfig;
};

} // namespace eerie_leap::configuration::traits

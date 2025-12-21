#pragma once

#include "cbor_trait.h"
#include <configuration/cbor/cbor_system_config/cbor_system_config.h>
#include "configuration/cbor/cbor_system_config/cbor_system_config_cbor_encode.h"
#include "configuration/cbor/cbor_system_config/cbor_system_config_cbor_decode.h"
#include "configuration/cbor/cbor_system_config/cbor_system_config_size.h"

namespace eerie_leap::configuration::cbor::traits {

template <>
struct CborTrait<CborSystemConfig> {
    static constexpr auto Encode = cbor_encode_CborSystemConfig;
    static constexpr auto Decode = cbor_decode_CborSystemConfig;
    static constexpr auto GetSerializingSize = cbor_get_size_CborSystemConfig;
};

} // namespace eerie_leap::configuration::cbor::traits

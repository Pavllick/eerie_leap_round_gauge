#pragma once

#include "cbor_trait.h"
#include <configuration/cbor/cbor_ui_config/cbor_ui_config.h>
#include "configuration/cbor/cbor_ui_config/cbor_ui_config_cbor_encode.h"
#include "configuration/cbor/cbor_ui_config/cbor_ui_config_cbor_decode.h"
#include "configuration/cbor/cbor_ui_config/cbor_ui_config_size.h"

namespace eerie_leap::configuration::cbor::traits {

template <>
struct CborTrait<CborUiConfig> {
    static constexpr auto Encode = cbor_encode_CborUiConfig;
    static constexpr auto Decode = cbor_decode_CborUiConfig;
    static constexpr auto GetSerializingSize = cbor_get_size_CborUiConfig;
};

} // namespace eerie_leap::configuration::cbor::traits

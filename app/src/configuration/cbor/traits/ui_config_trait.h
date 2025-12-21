#pragma once

#include "cbor_trait.h"
#include <configuration/cbor/ui_config/ui_config.h>
#include "configuration/cbor/ui_config/ui_config_cbor_encode.h"
#include "configuration/cbor/ui_config/ui_config_cbor_decode.h"
#include "configuration/cbor/ui_config/cbor_ui_config_size.h"

namespace eerie_leap::configuration::cbor::traits {

template <>
struct CborTrait<UiConfig> {
    static constexpr auto Encode = cbor_encode_UiConfig;
    static constexpr auto Decode = cbor_decode_UiConfig;
    static constexpr auto GetSerializingSize = cbor_get_size_CborUiConfig;
};

} // namespace eerie_leap::configuration::cbor::traits

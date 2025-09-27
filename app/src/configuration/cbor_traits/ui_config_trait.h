#pragma once

#include "cbor_trait.h"
#include <configuration/ui_config/ui_config.h>
#include "configuration/ui_config/ui_config_cbor_encode.h"
#include "configuration/ui_config/ui_config_cbor_decode.h"

namespace eerie_leap::configuration::traits {

template <>
struct CborTrait<UiConfig> {
    static constexpr auto Encode = cbor_encode_UiConfig;
    static constexpr auto Decode = cbor_decode_UiConfig;
};

} // namespace eerie_leap::configuration::traits

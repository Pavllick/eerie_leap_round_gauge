#pragma once

#include "cbor_trait.h"
#include <configuration/gauge_config/gauge_config.h>
#include "configuration/gauge_config/gauge_config_cbor_encode.h"
#include "configuration/gauge_config/gauge_config_cbor_decode.h"

namespace eerie_leap::configuration::traits {

template <>
struct CborTrait<GaugeConfig> {
    static constexpr auto Encode = cbor_encode_GaugeConfig;
    static constexpr auto Decode = cbor_decode_GaugeConfig;
};

} // namespace eerie_leap::configuration::traits

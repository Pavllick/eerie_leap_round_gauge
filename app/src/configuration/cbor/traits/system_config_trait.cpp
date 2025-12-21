#include "configuration/cbor/cbor_trait.h"
#include <configuration/cbor/cbor_system_config/cbor_system_config.h>
#include "configuration/cbor/cbor_system_config/cbor_system_config_cbor_encode.h"
#include "configuration/cbor/cbor_system_config/cbor_system_config_cbor_decode.h"
#include "configuration/cbor/cbor_system_config/cbor_system_config_size.h"

namespace eerie_leap::configuration::cbor::traits {

struct CborSystemConfigTraitRegistrar {
    CborSystemConfigTraitRegistrar() {
        CborTraitRegistry::Register<CborSystemConfig>(
            cbor_encode_CborSystemConfig,
            cbor_decode_CborSystemConfig,
            cbor_get_size_CborSystemConfig
        );
    }
} CborSystemConfigTraitRegistrar;

} // namespace eerie_leap::configuration::cbor::traits

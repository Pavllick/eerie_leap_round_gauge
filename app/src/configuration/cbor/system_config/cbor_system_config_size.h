#pragma once

#include "utilities/cbor/cbor_size_builder.hpp"

#include "system_config.h"

using namespace eerie_leap::utilities::cbor;

static size_t cbor_get_size_CborSystemConfig(const SystemConfig& config) {
    CborSizeBuilder builder;
    builder.AddIndefiniteArrayStart();

    builder.AddUint(config.device_id)
        .AddUint(config.hw_version)
        .AddUint(config.sw_version)
        .AddUint(config.build_number)
        .AddUint(config.interface_channel);

    return builder.Build();
}

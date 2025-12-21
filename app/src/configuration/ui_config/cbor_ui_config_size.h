#pragma once

#include "utilities/cbor/cbor_size_builder.hpp"

#include "ui_config.h"

using namespace eerie_leap::utilities::cbor;

static size_t GetPropertyValueTypeSize(const PropertyValueType_r& value) {
    CborSizeBuilder builder;

    switch (value.PropertyValueType_choice) {
        case PropertyValueType_r::PropertyValueType_int_c:
            builder.AddInt(std::get<int32_t>(value.value));
            break;
        case PropertyValueType_r::PropertyValueType_float_c:
            builder.AddDouble(std::get<double>(value.value));
            break;
        case PropertyValueType_r::PropertyValueType_tstr_c:
            builder.AddTstr(std::get<zcbor_string>(value.value));
            break;
        case PropertyValueType_r::PropertyValueType_bool_c:
            builder.AddBool(std::get<bool>(value.value));
            break;
        case PropertyValueType_r::PropertyValueType_int_l_c: {
            const auto& vec = std::get<std::vector<int32_t>>(value.value);
            builder.AddIndefiniteArrayStart();
            for(const auto& item : vec)
                builder.AddInt(item);

            break;
        }
        case PropertyValueType_r::PropertyValueType_tstr_l_c: {
            const auto& vec = std::get<std::vector<zcbor_string>>(value.value);
            builder.AddIndefiniteArrayStart();
            for(const auto& item : vec)
                builder.AddTstr(item);

            break;
        }
        case PropertyValueType_r::PropertyValueType_map_c: {
            const auto& vec = std::get<std::vector<map_tstrtstr>>(value.value);
            builder.AddIndefiniteArrayStart();
            for(const auto& item : vec) {
                builder.AddTstr(item.tstrtstr_key)
                    .AddTstr(item.tstrtstr);
            }
            break;
        }
        default:
            break;
    }

    return builder.Build();
}

static size_t cbor_get_size_CborUiConfig(const UiConfig& config) {
    CborSizeBuilder builder;
    builder.AddIndefiniteArrayStart();

    builder.AddUint(config.version)
        .AddUint(config.active_screen_index);

    builder.AddOptional(config.properties_present,
        config.properties,
        [](const PropertiesConfig& properties) {

        CborSizeBuilder builder;
        builder.AddIndefiniteArrayStart();

        for(const auto& property : properties.PropertyValueType_m) {
            builder.AddTstr(property.PropertyValueType_m_key)
                .AddSize(GetPropertyValueTypeSize(property.PropertyValueType_m));
        }

        return builder.Build();
    });

    builder.AddIndefiniteArrayStart();
    for(const auto& screen : config.ScreenConfig_m) {
        builder.AddIndefiniteArrayStart();

        builder.AddUint(screen.id)
            .AddUint(screen.type);

        // GridSettingsConfig
        builder.AddIndefiniteArrayStart();
        builder.AddBool(screen.grid.snap_enabled)
            .AddUint(screen.grid.width)
            .AddUint(screen.grid.height)
            .AddUint(screen.grid.spacing_px);

        builder.AddIndefiniteArrayStart();
        for(const auto& widget : screen.WidgetConfig_m) {
            builder.AddIndefiniteArrayStart();

            builder.AddUint(widget.type)
                .AddUint(widget.id);

            // WidgetPositionConfig
            builder.AddIndefiniteArrayStart();
            builder.AddInt(widget.position.x)
                .AddInt(widget.position.y);

            // WidgetSizeConfig
            builder.AddIndefiniteArrayStart();
            builder.AddUint(widget.size.width)
                .AddUint(widget.size.height);

            builder.AddOptional(widget.properties_present,
                widget.properties,
                [](const PropertiesConfig& properties) {

                CborSizeBuilder builder;
                builder.AddIndefiniteArrayStart();

                for(const auto& property : properties.PropertyValueType_m) {
                    builder.AddTstr(property.PropertyValueType_m_key)
                        .AddSize(GetPropertyValueTypeSize(property.PropertyValueType_m));
                }

                return builder.Build();
            });
        }
    }

    return builder.Build();
}

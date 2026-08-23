#pragma once

#include <optional>
#include <string_view>
#include <variant>

#include "utilities/type/config_value.h"

namespace eerie_leap::domain::ui_domain::services {

using eerie_leap::utilities::type::ConfigValue;

struct SettingRange {
    double min = 0;
    double max = 0;
    double step = 0;
};

// A device setting a control widget can bind to by id, without ever touching a driver.
//
// Control widgets call these from the LVGL renderer thread with the LVGL lock
// held, so an implementation - and every binding behind it - must not block.
// Commit() in particular has to hand the flash write to a work queue rather
// than perform it inline.
class ISettingsProvider {
public:
    virtual ~ISettingsProvider() = default;

    virtual std::optional<ConfigValue> Get(std::string_view setting_id) const = 0;
    virtual int Set(std::string_view setting_id, const ConfigValue& value) = 0;

    // Persists the value applied by Set(); called on release so dragging a
    // slider does not write flash on every tick.
    virtual int Commit(std::string_view setting_id) = 0;

    virtual std::optional<SettingRange> GetRange(std::string_view setting_id) const = 0;
};

inline std::optional<double> ToSettingNumber(const ConfigValue& value) {
    if(const auto* int_value = std::get_if<int>(&value))
        return static_cast<double>(*int_value);

    if(const auto* double_value = std::get_if<double>(&value))
        return *double_value;

    return std::nullopt;
}

inline std::optional<bool> ToSettingBoolean(const ConfigValue& value) {
    if(const auto* bool_value = std::get_if<bool>(&value))
        return *bool_value;

    return std::nullopt;
}

} // namespace eerie_leap::domain::ui_domain::services

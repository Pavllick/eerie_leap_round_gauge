#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <zephyr/kernel.h>

#include "domain/settings_domain/utilities/i_settings_provider.h"

namespace eerie_leap::domain::settings_domain::utilities {

// Resolves setting ids onto the domain services that own them.
class SettingsRegistry : public ISettingsProvider {
public:
    struct Binding {
        std::function<ConfigValue()> get;
        std::function<int(const ConfigValue&)> set;
        std::function<int()> commit;
        SettingRange range;
    };

private:
    struct TransparentHash {
        using is_transparent = void;

        size_t operator()(std::string_view value) const {
            return std::hash<std::string_view>{}(value);
        }
    };

    std::unordered_map<std::string, Binding, TransparentHash, std::equal_to<>> bindings_;
    mutable k_mutex lock_;

    // Bindings are copied out before they run: they reach drivers and work
    // queues, and holding the registry locked across that would put every other
    // reader - including the LVGL renderer thread - behind them.
    std::optional<Binding> Find(std::string_view setting_id) const;

public:
    SettingsRegistry();
    SettingsRegistry(const SettingsRegistry&) = delete;
    SettingsRegistry& operator=(const SettingsRegistry&) = delete;

    int Register(std::string setting_id, Binding binding);

    // Bindings capture the service that owns them, so an owner that is torn down
    // has to withdraw them first.
    int Unregister(std::string_view setting_id);

    std::optional<ConfigValue> Get(std::string_view setting_id) const override;
    int Set(std::string_view setting_id, const ConfigValue& value) override;
    int Commit(std::string_view setting_id) override;
    std::optional<SettingRange> GetRange(std::string_view setting_id) const override;
};

} // namespace eerie_leap::domain::settings_domain::utilities

#pragma once

#include <memory>
#include <memory_resource>
#include <optional>

#include "domain/settings_domain/utilities/i_settings_provider.h"

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets {

using eerie_leap::domain::settings_domain::utilities::SettingRange;
using eerie_leap::utilities::type::ConfigValue;

// Binds a widget to a setting id resolved through ISettingsProvider, so it never
// reaches a driver or the configuration layer directly. Read-only on its own -
// ControlBase adds the interactive half, indicators bind straight to this.
class SettingWidgetBase : public WidgetBase {
protected:
    std::pmr::string setting_id_;
    std::optional<SettingRange> range_;

    SettingWidgetBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context);

    // Bindings can be registered after the UI is configured, so an absent range
    // is re-resolved rather than cached as "unbound" for the widget's lifetime.
    void RefreshRange();

    virtual void OnRangeResolved();
    virtual void OnSettingChanged();

    bool HasSetting() const;
    std::optional<ConfigValue> GetSettingValue() const;
    int SetSettingValue(const ConfigValue& value);
    int CommitSetting();

public:
    ~SettingWidgetBase() = default;

protected:
    void RegisterProperties(WidgetPropertyStore& store) override;
    void OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) override;
    void OnConfigured() override;

    // Settings only publish on change, so a widget hidden while one moved has no
    // other chance to catch up.
    void OnActivated() override;
};

} // namespace eerie_leap::views::widgets

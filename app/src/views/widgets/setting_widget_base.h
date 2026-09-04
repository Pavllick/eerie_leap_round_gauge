#pragma once

#include <memory>
#include <memory_resource>

#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets {

using eerie_leap::utilities::type::ConfigValue;

// Binds a widget to a setting id over SettingsEventsChannel. It never reaches a driver, a
// provider or the configuration layer: the owner of the setting publishes what it applied,
// and the widget asks for changes it may not get. Read-only on its own - ControlBase adds
// the interactive half, indicators bind straight to this.
class SettingWidgetBase : public WidgetBase {
protected:
    std::pmr::string setting_id_;
    uint32_t setting_id_hash_ = 0;

    SettingWidgetBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context);

    bool HasSetting() const;

    // Publishes a request; what the widget ends up showing is whatever comes back on
    // Changed, which may be clamped, quantized, or nothing at all.
    void RequestSettingValue(const ConfigValue& value) const;

    // Owners only publish on change, so a widget created or revealed after the fact has to ask.
    void RequestSettingState() const;

public:
    ~SettingWidgetBase() = default;

protected:
    void RegisterProperties(WidgetPropertyStore& store) override;
    void OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) override;
    void OnConfigured() override;
    void OnActivated() override;
};

} // namespace eerie_leap::views::widgets

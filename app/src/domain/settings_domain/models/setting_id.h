#pragma once

namespace eerie_leap::domain::settings_domain::models {

// Keys a SettingsRegistry binding is registered under and a widget's SETTING_ID
// property refers to. Persisted inside widget configurations, so treat them as
// a wire format: add, never rename.
class SettingId {
public:
    static constexpr const char* DISPLAY_BRIGHTNESS = "display.brightness";
};

} // namespace eerie_leap::domain::settings_domain::models

#pragma once

#include "views/themes/i_theme.h"

namespace eerie_leap::views::themes {

class DarkTheme : public ITheme {
public:
    Color GetPrimaryColor() const override {
        return Color(0x64B5F6); // Light blue
    }

    Color GetSecondaryColor() const override {
        return Color(0xF56060); // Red
    }

    Color GetInactiveColor() const override {
        return Color(0x909090); // Light gray
    }

    Color GetBackgroundColor() const override {
        return Color(0x000000); // Almost black
    }

    Color GetSurfaceColor() const override {
        return Color(0x1E1E1E); // Dark gray
    }

    Color GetAccentColor() const override {
        return Color(0xF56060); // Red
    }

    Color GetErrorColor() const override {
        return Color(0xEF5350); // Light red
    }

    Color GetTextColor() const override {
        return Color(0xE0E0E0); // Light gray
    }

    Color GetTextSecondaryColor() const override {
        return Color(0xFFFFFF); // White
    }

    Font GetPrimaryFont() const override {
        return Font("Montserrat", &lv_font_montserrat_20);
    }

    Font GetSecondaryFont() const override {
        return Font("Montserrat", &lv_font_montserrat_20);
    }
};

} // namespace eerie_leap::views::themes

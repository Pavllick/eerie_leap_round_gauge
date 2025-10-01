#pragma once

#include "views/themes/i_theme.h"

namespace eerie_leap::views::themes {

class DefaultTheme : public ITheme {
public:
    Color GetPrimaryColor() const override {
        return Color(0x2196F3); // Blue
    }

    Color GetSecondaryColor() const override {
        return Color(0xF56060); // Red
    }

    Color GetInactiveColor() const override {
        return Color(0x919191); // Medium gray
    }

    Color GetBackgroundColor() const override {
        return Color(0xFFFFFF); // White
    }

    Color GetSurfaceColor() const override {
        return Color(0xF5F5F5); // Light gray
    }

    Color GetAccentColor() const override {
        return Color(0xF56060); // Red
    }

    Color GetErrorColor() const override {
        return Color(0xF44336); // Red
    }

    Color GetTextColor() const override {
        return Color(0x212121); // Dark gray
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

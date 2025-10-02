#pragma once

#include "views/themes/i_theme.h"

namespace eerie_leap::views::themes {

class DarkTheme : public ITheme {
public:
    Color GetPrimaryColor() const override {
        return Color(0xFAFAFA); // Light gray
    }

    Color GetSecondaryColor() const override {
        return Color(0x1BBE5F); // Green
    }

    Color GetInactiveColor() const override {
        return Color(0x909090); // Light gray
    }

    Color GetBackgroundColor() const override {
        return Color(0x000000); // Black
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

    Font GetPrimaryFont() const override {
        return Font("Montserrat", &lv_font_montserrat_20, 20);
    }

    Font GetSecondaryFont() const override {
        return Font("Montserrat", &lv_font_montserrat_20, 20);
    }

    Font GetPrimaryFontLarge() const override {
        return Font("Inconsolata", &lv_font_inconsolata_bold_120, 120);
    }
};

} // namespace eerie_leap::views::themes

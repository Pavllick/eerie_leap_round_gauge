#pragma once

#include "views/themes/i_theme.h"

namespace eerie_leap::views::themes {

// using namespace eerie_leap::views::assets::fonts;

class DefaultTheme : public ITheme {
public:
    Color GetPrimaryColor() const override {
        return Color(0x0F0F0F); // Dark gray
    }

    Color GetSecondaryColor() const override {
        return Color(0x1BBE5F); // Green
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

    Font GetPrimaryFont() const override {
        return Font("Montserrat", &lv_font_rubik_medium_20, 20);
    }

    Font GetSecondaryFont() const override {
        return Font("Montserrat", &lv_font_montserrat_20, 20);
    }

    Font GetPrimaryFontLarge() const override {
        return Font("Inconsolata", &lv_font_inconsolata_bold_120, 120);
    }
};

} // namespace eerie_leap::views::themes

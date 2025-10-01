#pragma once

#include "views/themes/i_theme.h"

namespace eerie_leap::views::themes {

class DarkTheme : public ITheme {
public:
    Color GetPrimaryColor() const override {
        return Color(0x64B5F6); // Light blue
    }

    Color GetSecondaryColor() const override {
        return Color(0x909090); // Light gray
    }

    Color GetBackgroundColor() const override {
        return Color(0x121212); // Almost black
    }

    Color GetSurfaceColor() const override {
        return Color(0x1E1E1E); // Dark gray
    }

    Color GetTextColor() const override {
        return Color(0xE0E0E0); // Light gray
    }

    Color GetTextSecondaryColor() const override {
        return Color(0x909090); // Medium gray
    }

    Color GetAccentColor() const override {
        return Color(0xFFB74D); // Light orange
    }

    Color GetErrorColor() const override {
        return Color(0xEF5350); // Light red
    }

    // Spacing GetDefaultSpacing() const override {
    //     return Spacing{8, 8, 1};
    // }

    // Typography GetTypography() const override {
    //     return Typography{14, 18, 12};
    // }

    // uint8_t GetBorderRadius() const override {
    //     return 4;
    // }
};

} // namespace eerie_leap::views::themes

#pragma once

#include "views/themes/i_theme.h"

namespace eerie_leap::views::themes {

class DefaultTheme : public ITheme {
public:
    Color GetPrimaryColor() const override {
        return Color(0x2196F3); // Blue
    }

    Color GetSecondaryColor() const override {
        return Color(0x757575); // Gray
    }

    Color GetBackgroundColor() const override {
        return Color(0xFFFFFF); // White
    }

    Color GetSurfaceColor() const override {
        return Color(0xF5F5F5); // Light gray
    }

    Color GetTextColor() const override {
        return Color(0x212121); // Dark gray
    }

    Color GetTextSecondaryColor() const override {
        return Color(0x757575); // Medium gray
    }

    Color GetAccentColor() const override {
        return Color(0xFF9800); // Orange
    }

    Color GetErrorColor() const override {
        return Color(0xF44336); // Red
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

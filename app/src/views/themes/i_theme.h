#pragma once

#include "views/themes/types/color.h"

namespace eerie_leap::views::themes {

using namespace eerie_leap::views::themes::types;

class ITheme {
public:
    virtual ~ITheme() = default;

    // Colors
    virtual Color GetPrimaryColor() const = 0;
    virtual Color GetSecondaryColor() const = 0;
    virtual Color GetBackgroundColor() const = 0;
    virtual Color GetSurfaceColor() const = 0;
    virtual Color GetTextColor() const = 0;
    virtual Color GetTextSecondaryColor() const = 0;
    virtual Color GetAccentColor() const = 0;
    virtual Color GetErrorColor() const = 0;

    // // Spacing
    // virtual Spacing GetDefaultSpacing() const = 0;

    // // Typography
    // virtual Typography GetTypography() const = 0;

    // // Border radius
    // virtual uint8_t GetBorderRadius() const = 0;
};

} // namespace eerie_leap::views::themes

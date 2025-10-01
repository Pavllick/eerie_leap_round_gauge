#pragma once

#include "views/themes/types/color.h"
#include "views/themes/types/font.h"

namespace eerie_leap::views::themes {

using namespace eerie_leap::views::themes::types;

class ITheme {
public:
    virtual ~ITheme() = default;

    // Colors
    virtual Color GetPrimaryColor() const = 0;
    virtual Color GetSecondaryColor() const = 0;
    virtual Color GetInactiveColor() const = 0;
    virtual Color GetBackgroundColor() const = 0;
    virtual Color GetSurfaceColor() const = 0;
    virtual Color GetAccentColor() const = 0;
    virtual Color GetErrorColor() const = 0;

    virtual Font GetPrimaryFont() const = 0;
    virtual Font GetSecondaryFont() const = 0;
};

} // namespace eerie_leap::views::themes

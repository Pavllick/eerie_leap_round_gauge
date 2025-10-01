#pragma once

namespace eerie_leap::views::themes {

class IThemeObserver {
public:
    virtual ~IThemeObserver() = default;
    virtual void OnThemeChanged() = 0;
};

} // namespace eerie_leap::views::themes

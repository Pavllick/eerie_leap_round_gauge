#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

// What an input source asks the navigation to do.
enum class NavigationIntent : uint32_t {
    None = 0,
    NextGroup,
    PreviousGroup,
    GoToGroup,
    Back,
    ShowOverlay,
    CloseOverlay
};

// What the view layer is told to do, published on the UI event bus.
enum class NavigationAction : uint32_t {
    None = 0,
    ShowGroup,
    ShowOverlay,
    CloseOverlay
};

} // namespace eerie_leap::domain::ui_domain::models

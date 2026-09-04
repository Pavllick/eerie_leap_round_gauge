#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

// Persisted as part of a widget binding - append only.
enum class PropertyBindingDirection : uint32_t {
    In = 0,
    Out,
    InOut
};

} // namespace eerie_leap::domain::ui_domain::models

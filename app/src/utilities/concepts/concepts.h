#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>

namespace eerie_leap::utilities::concepts {

template<typename T>
concept EnumClassUint32 = std::is_scoped_enum_v<T>
    && std::same_as<std::underlying_type_t<T>, uint32_t>;

} // namespace eerie_leap::utilities::concepts

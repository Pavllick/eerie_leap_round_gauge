#pragma once

#include <cstdint>
#include <optional>

namespace eerie_leap::views::widgets::indicators {

class IIndicator {
public:
    virtual std::optional<size_t> GetSensorIdHash() const = 0;
    virtual void Update(float value) = 0;
};

} // namespace eerie_leap::views::widgets::indicators

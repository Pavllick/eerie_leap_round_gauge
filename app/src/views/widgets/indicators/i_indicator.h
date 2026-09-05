#pragma once

namespace eerie_leap::views::widgets::indicators {

class IIndicator {
public:
    virtual void Update(float value) = 0;
};

} // namespace eerie_leap::views::widgets::indicators

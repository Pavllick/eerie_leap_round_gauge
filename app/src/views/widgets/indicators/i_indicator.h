#pragma once

#include <optional>

#include "views/widgets/indicators/indicator_state.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::views::widgets;

class IIndicator : public IWidget {
public:
    virtual std::optional<uint32_t> GetSensorId() const = 0;
    virtual void Update(float value) = 0;
    virtual const IndicatorState* GetState() const = 0;
};

} // namespace eerie_leap::views::widgets::indicators

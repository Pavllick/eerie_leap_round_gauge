#pragma once

#include <lvgl.h>

#include "views/screens/radial_digital_gauge/radial_digital_gauge.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::screens;

class MainView {
private:
    RadialDigitalGauge radial_digital_gauge_;

    void RenderBackground();

public:
    MainView();

    int Render();
    void Update(float value);
};

} // namespace eerie_leap::views

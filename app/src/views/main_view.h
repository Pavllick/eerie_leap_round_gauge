#pragma once

#include <memory>
#include <unordered_map>

#include <lvgl.h>

#include "views/screens/i_screen.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::screens;

class MainView {
private:
    std::unordered_map<uint32_t, std::unique_ptr<IScreen>> screens_;

    void RenderBackground();

public:
    MainView();

    void AddScreen(uint32_t id, std::unique_ptr<IScreen> screen);
    int Render();
};

} // namespace eerie_leap::views

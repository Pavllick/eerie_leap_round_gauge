#pragma once

#include <memory>
#include <unordered_map>

#include <lvgl.h>

#include "views/screens/i_screen.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::screens;

class MainView {
private:
    std::unordered_map<uint32_t, std::shared_ptr<IScreen>> screens_;
    uint32_t active_screen_id_;

    void RenderBackground();

public:
    MainView();

    void AddScreen(uint32_t id, std::shared_ptr<IScreen> screen);
    int SetActiveScreen(uint32_t id);
    std::shared_ptr<IScreen> GetScreen(uint32_t id);
    std::shared_ptr<IScreen> GetActiveScreen();

    int Render();
};

} // namespace eerie_leap::views

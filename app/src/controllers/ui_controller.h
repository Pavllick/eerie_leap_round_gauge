#pragma once

#include <memory>

#include "domain/interface_domain/interface.h"
#include "views/main/main_view.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::views::main;

class UiController {
private:
    std::shared_ptr<Interface> interface_;
    std::shared_ptr<MainView> main_view_;

public:
    UiController(std::shared_ptr<Interface> interface, std::shared_ptr<MainView> main_view);

    int Initialize();
};

} // namespace eerie_leap::controllers

#pragma once

namespace eerie_leap::subsys::device_tree {

class DtConfigurator {
private:
    DtConfigurator() = default;

public:
    static void Initialize();
};

} // namespace eerie_leap::subsys::device_tree

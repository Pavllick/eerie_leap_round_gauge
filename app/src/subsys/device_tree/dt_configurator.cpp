#include "dt_display.h"
#include "dt_fs.h"
#include "dt_canbus.h"
#include "dt_gpio.h"

#include "dt_configurator.h"

namespace eerie_leap::subsys::device_tree {

void DtConfigurator::Initialize() {
    DtDisplay::Initialize();
    DtFs::InitInternalFs();
    DtFs::InitSdFs();
    DtCanbus::Initialize();
    DtGpio::Initialize();
}

} // namespace eerie_leap::subsys::device_tree

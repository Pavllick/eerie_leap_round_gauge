#include <memory>
#include <ctime>
#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "utilities/memory/heap_allocator.h"
#include "utilities/dev_tools/system_info.h"
#include "utilities/guid/guid_generator.h"
// #include "utilities/time/boot_elapsed_time_service.h"

#include "subsys/modbus/modbus.h"

#include "domain/device_tree/device_tree_setup.h"
#include "domain/interface_domain/interface.h"
#include "domain/ui_domain/ui_renderer.h"
#include "domain/sensor_domain/services/reading_processor_service.h"
// #include "domain/fs_domain/services/fs_service.h"

#include "views/main/main_view.h"

#include "controllers/ui_controller.h"

// #include "configuration/system_config/system_config.h"
// #include "configuration/services/configuration_service.h"
// #include "controllers/system_configuration_controller.h"

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
using namespace eerie_leap::utilities::guid;
// using namespace eerie_leap::utilities::time;

// using namespace eerie_leap::controllers;

using namespace eerie_leap::subsys::modbus;

using namespace eerie_leap::domain::device_tree;
using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::ui_domain;
using namespace eerie_leap::domain::sensor_domain::services;
// using namespace eerie_leap::domain::fs_domain::services;
// using namespace eerie_leap::configuration::services;

using namespace eerie_leap::views::main;

using namespace eerie_leap::controllers;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 2000;
const size_t DISPLAY_WIDTH = 466;
const size_t DISPLAY_HEIGHT = 466;

int main()
{
    DeviceTreeSetup::Initialize();
    auto& device_tree_setup = DeviceTreeSetup::Get();

    auto guid_generator = make_shared_ext<GuidGenerator>();

    auto reading_processor_service = make_shared_ext<ReadingProcessorService>();
    reading_processor_service->Initialize();

    auto modbus = make_shared_ext<Modbus>(device_tree_setup.GetModbusIface().value(), 1);
    auto interface = make_shared_ext<Interface>(modbus, guid_generator, reading_processor_service);
    if(interface->Initialize() != 0)
        return -1;

    auto main_view = make_shared_ext<MainView>();

    auto ui_controller = make_shared_ext<UiController>(reading_processor_service, interface, main_view);
    if(ui_controller->Initialize({ 2348664336 }) != 0)
        return -1;

    auto ui_renderer = make_shared_ext<UiRenderer>();
    if(ui_renderer->Initialize() != 0)
        return -1;

    ui_renderer->Start();

	while (true) {
        k_msleep(SLEEP_TIME_MS);

        // SystemInfo::print_heap_info();
        // SystemInfo::print_stack_info();
	}

	return 0;
}

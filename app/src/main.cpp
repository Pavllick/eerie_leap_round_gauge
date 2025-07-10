#include <memory>
#include <ctime>
#include <zephyr/logging/log.h>

#include "utilities/memory/heap_allocator.h"
#include "utilities/dev_tools/system_info.h"
#include "utilities/time/boot_elapsed_time_service.h"
#include "domain/fs_domain/services/fs_service.h"

#include "configuration/system_config/system_config.h"
#include "configuration/services/configuration_service.h"
#include "controllers/system_configuration_controller.h"

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
using namespace eerie_leap::utilities::time;

using namespace eerie_leap::controllers;

using namespace eerie_leap::domain::fs_domain::services;
using namespace eerie_leap::configuration::services;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 10000;

int main() {

    auto fs_service = make_shared_ext<FsService>();

    auto time_service = make_shared_ext<BootElapsedTimeService>();
    time_service->Initialize();

    auto system_config_service = make_shared_ext<ConfigurationService<SystemConfig>>("system_config", fs_service);
    auto system_configuration_controller = make_shared_ext<SystemConfigurationController>(system_config_service);

    while (true) {
        SystemInfo::print_heap_info();
        SystemInfo::print_stack_info();
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}

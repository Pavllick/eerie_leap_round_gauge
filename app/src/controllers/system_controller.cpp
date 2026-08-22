#include <zephyr/logging/log.h>
#include <eerie_memory.hpp>

#include "utilities/memory/memory_resource_manager.h"

#include "configuration/cbor/cbor_system_config/cbor_system_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "system_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;

namespace config_services = eerie_leap::configuration::services;

LOG_MODULE_REGISTER(system_controller_logger);

SystemController::SystemController(
    std::shared_ptr<IFsService> fs_service,
    std::shared_ptr<WorkQueueThread> config_work_queue_thread)
    : fs_service_(std::move(fs_service)),
      config_work_queue_thread_(std::move(config_work_queue_thread)) {}

int SystemController::Initialize() {
    auto system_config_service = std::make_unique<config_services::CborConfigurationService<CborSystemConfig>>(
        SYSTEM_CONFIGURATION_NAME, fs_service_, config_work_queue_thread_);
    system_configuration_manager_ = make_shared_pmr<SystemConfigurationManager>(
        Mrm::GetExtPmr(), std::move(system_config_service));

    return 0;
}

} // namespace eerie_leap::controllers

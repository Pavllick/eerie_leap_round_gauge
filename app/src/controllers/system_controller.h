#pragma once

#include <memory>

#include "subsys/fs/services/i_fs_service.h"
#include "subsys/threading/work_queue_thread.h"

#include "domain/system_domain/configuration/system_configuration_manager.h"

namespace eerie_leap::controllers {

using eerie_leap::subsys::fs::services::IFsService;
using eerie_leap::subsys::threading::WorkQueueThread;

using eerie_leap::domain::system_domain::configuration::SystemConfigurationManager;

class SystemController {
private:
    static constexpr const char* SYSTEM_CONFIGURATION_NAME = "system_config";

    std::shared_ptr<IFsService> fs_service_;
    std::shared_ptr<WorkQueueThread> config_work_queue_thread_;

    std::shared_ptr<SystemConfigurationManager> system_configuration_manager_;

public:
    SystemController(
        std::shared_ptr<IFsService> fs_service,
        std::shared_ptr<WorkQueueThread> config_work_queue_thread);

    int Initialize();

    std::shared_ptr<SystemConfigurationManager> GetConfigurationManager() const { return system_configuration_manager_; }
};

} // namespace eerie_leap::controllers

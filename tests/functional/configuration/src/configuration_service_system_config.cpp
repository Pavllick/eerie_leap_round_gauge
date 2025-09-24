#include <memory>
#include <zephyr/ztest.h>

#include "utilities/cbor/cbor_helpers.hpp"
#include "configuration/system_config/system_config.h"
#include "configuration/services/configuration_service.h"

#include "subsys/device_tree/dt_fs.h"
#include "subsys/fs/services/i_fs_service.h"
#include "subsys/fs/services/fs_service.h"

using namespace eerie_leap::utilities::cbor;
using namespace eerie_leap::configuration::services;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::subsys::device_tree;

ZTEST_SUITE(configuration_service_system_config, NULL, NULL, NULL, NULL, NULL);

ZTEST(configuration_service_system_config, test_SystemConfig_Save_config_successfully_saved) {
    SystemConfig system_config;
    memset(&system_config, 0, sizeof(system_config));

    system_config.device_id = 12;
    system_config.hw_version = 22;
    system_config.sw_version = 2422;

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto system_config_service = std::make_shared<ConfigurationService<SystemConfig>>("system_config", fs_service);

    auto save_res = system_config_service->Save(&system_config);
    zassert_true(save_res);
}

ZTEST(configuration_service_system_config, test_SystemConfig_Load_config_successfully_saved_and_loaded) {
    SystemConfig system_config;
    memset(&system_config, 0, sizeof(system_config));

    system_config.device_id = 14;
    system_config.hw_version = 46;
    system_config.sw_version = 8624;

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto system_config_service = std::make_shared<ConfigurationService<SystemConfig>>("system_config", fs_service);

    auto save_res = system_config_service->Save(&system_config);
    zassert_true(save_res);

    auto loaded_config = system_config_service->Load();
    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->device_id, 14);
    zassert_equal(loaded_config.value().config->hw_version, 46);
    zassert_equal(loaded_config.value().config->sw_version, 8624);
}

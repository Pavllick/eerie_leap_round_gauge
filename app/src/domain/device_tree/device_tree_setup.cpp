#include <filesystem>

#include <zephyr/logging/log.h>

#include "device_tree_setup.h"

namespace eerie_leap::domain::device_tree {

LOG_MODULE_REGISTER(dt_setup_logger);

std::optional<fs_mount_t> DeviceTreeSetup::int_fs_mp_ = std::nullopt;
std::optional<fs_mount_t> DeviceTreeSetup::sd_fs_mp_ = std::nullopt;
std::optional<char*> DeviceTreeSetup::modbus_iface_ = std::nullopt;

void DeviceTreeSetup::Initialize() {
    InitInternalFs();
    InitSdFs();
    InitModbus();
}

DeviceTreeSetup& DeviceTreeSetup::Get() {
    static DeviceTreeSetup instance;
    return instance;
}

void DeviceTreeSetup::InitInternalFs() {
    // int_fs_mp_ = std::make_optional<fs_mount_t>(FS_FSTAB_ENTRY(INT_FS_NODE));
    // LOG_INF("Internal File System initialized.");
}

void DeviceTreeSetup::InitSdFs() {
#if DT_HAS_ALIAS(sdhc0)
    static std::string disk_name = CONFIG_EERIE_LEAP_SD_DRIVE_NAME;

    static std::string mount_point = CONFIG_EERIE_LEAP_SD_MOUNT_POINT;
    std::filesystem::path full_path(mount_point);
    mount_point = full_path.string();

    sd_fs_mp_ = {
        .type = FS_LITTLEFS,
        .mnt_point = mount_point.c_str(),
        .storage_dev = (void *)disk_name.c_str(),
        .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,
    };
    LOG_INF("SD File System initialized.");
#endif
}

void DeviceTreeSetup::InitModbus() {
#if DT_HAS_ALIAS(modbus0)
    modbus_iface_ = std::make_optional<char*>(DEVICE_DT_NAME(MODBUS_NODE));
    LOG_INF("Modbus initialized.");
#endif
}

} // namespace eerie_leap::domain::device_tree

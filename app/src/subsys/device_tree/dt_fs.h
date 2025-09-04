#pragma once

#include <vector>
#include <optional>

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#ifdef CONFIG_SDMMC_SUBSYS
#include <zephyr/drivers/sdhc.h>
#endif
#include <zephyr/fs/fs.h>

// Internal File System
#if DT_HAS_ALIAS(intfs0)
#define INT_FS_NODE DT_ALIAS(intfs0)
FS_FSTAB_DECLARE_ENTRY(INT_FS_NODE);
#endif

// SD File System
#if DT_HAS_ALIAS(sdhc0) && DT_HAS_ALIAS(sdfs0)
#define SDHC_NODE DT_ALIAS(sdhc0)
#define SD_DEV DEVICE_DT_GET(SDHC_NODE)
#define SD_FS_NODE DT_ALIAS(sdfs0)
FS_FSTAB_DECLARE_ENTRY(SD_FS_NODE);

#define SDHC_DISK_NAME(node_id) DT_PROP_OR(node_id, disk_name, "dfdf")
#endif

namespace eerie_leap::subsys::device_tree {

class DtFs {
private:
    static std::optional<fs_mount_t> int_fs_mp_;

    static std::optional<fs_mount_t> sd_fs_mp_;
#ifdef CONFIG_SDMMC_SUBSYS
    static sdhc_host_props sd_host_props_;
#endif
    static uint32_t sd_relative_addr_;
    static const char* sd_disk_name_;

    DtFs() = default;

#ifdef CONFIG_SDMMC_SUBSYS
    static int SdmmcRequestRca(const struct device* dev);
    static bool SdmmcReadStatus(const struct device* dev);
#endif

public:
    static void InitInternalFs();

    static void InitSdFs();
    static bool IsSdCardPresent();

    static std::optional<fs_mount_t>& GetInternalFsMp() { return int_fs_mp_; }
    static std::optional<fs_mount_t>& GetSdFsMp() { return sd_fs_mp_; }
    static const char* GetSdDiskName() { return sd_disk_name_; }
};

} // namespace eerie_leap::subsys::device_tree

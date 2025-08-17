#pragma once

#include <vector>
#include <optional>

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>

// // File System
// #define INT_FS_NODE DT_ALIAS(lfs1)
// FS_FSTAB_DECLARE_ENTRY(INT_FS_NODE);

// Modbus
#if DT_HAS_ALIAS(modbus0)
#define MODBUS_NODE DT_ALIAS(modbus0)
#endif

namespace eerie_leap::domain::device_tree {

class DeviceTreeSetup {
private:
    static std::optional<fs_mount_t> int_fs_mp_;
    static std::optional<fs_mount_t> sd_fs_mp_;
    static std::optional<char*> modbus_iface_;

    DeviceTreeSetup() = default;

    static void InitInternalFs();
    static void InitSdFs();
    static void InitModbus();
    static void InitGpio();
    static void InitAdc();

public:
    static DeviceTreeSetup& Get();
    static void Initialize();

    static std::optional<fs_mount_t>& GetInternalFsMp() { return int_fs_mp_; }
    static std::optional<fs_mount_t>& GetSdFsMp() { return sd_fs_mp_; }
    static std::optional<char*>& GetModbusIface() { return modbus_iface_; }
};

} // namespace eerie_leap::domain::device_tree

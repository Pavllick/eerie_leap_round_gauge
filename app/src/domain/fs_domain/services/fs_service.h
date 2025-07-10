#pragma once

#include <string>
#include <vector>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include "i_fs_service.h"

#define PARTITION_NODE DT_ALIAS(lfs1)
#define MKFS_DEV_ID FIXED_PARTITION_ID(lfs1_partition)
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);

namespace eerie_leap::domain::fs_domain::services {

class FsService : public IFsService {
private:
    struct fs_mount_t* mountpoint_;
    bool is_mounted_;

    bool Mount();
    void Unmount();

public:
    FsService() : mountpoint_(&FS_FSTAB_ENTRY(PARTITION_NODE)), is_mounted_(true) {}

    bool Initialize() override;
    bool WriteFile(const std::string& relative_path, const void* data_p, size_t data_size) override;
    bool ReadFile(const std::string& relative_path, void* data_p, size_t data_size, size_t& out_len) override;
    bool CreateDirectory(const std::string& relative_path) override;
    bool Exists(const std::string& relative_path) override;
    bool DeleteFile(const std::string& relative_path) override;
    bool DeleteRecursive(const std::string& relative_path = "") override;
    std::vector<std::string> ListFiles(const std::string& relative_path = "") const override;
    size_t GetTotalSpace() const override;
    size_t GetUsedSpace() const override;
    bool Format() override;
};

}

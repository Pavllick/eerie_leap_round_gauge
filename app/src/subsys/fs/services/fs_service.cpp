#include <sstream>
#include <filesystem>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/disk_access.h>

#include "fs_service.h"

namespace eerie_leap::subsys::fs::services {

LOG_MODULE_REGISTER(fs_service_logger);

FsService::FsService(fs_mount_t mountpoint) : mountpoint_(mountpoint) { }

bool FsService::Initialize() {
    if(!IsMounted())
        return Mount();

    return true;
}

bool FsService::WriteFile(const std::string& relative_path, const void* data_p, size_t data_size) {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return false;
    }

    std::filesystem::path full_path(mountpoint_.mnt_point);
    full_path /= relative_path;

    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, full_path.string().c_str(), FS_O_WRITE | FS_O_CREATE | FS_O_TRUNC);
    if(rc < 0) {
        LOG_ERR("fs_open failed: %d.", rc);
        return false;
    }

    rc = fs_write(&file, data_p, data_size);
    fs_close(&file);

    if(rc < 0) {
        LOG_ERR("fs_write failed: %d.", rc);
        return false;
    }

    return true;
}

bool FsService::ReadFile(const std::string& relative_path, void* data_p, size_t data_size, size_t& out_len) {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return false;
    }

    std::filesystem::path full_path(mountpoint_.mnt_point);
    full_path /= relative_path;

    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, full_path.string().c_str(), FS_O_READ);
    if(rc < 0) {
        LOG_ERR("fs_open failed: %d.", rc);
        return false;
    }

    rc = fs_read(&file, data_p, data_size);
    fs_close(&file);

    if(rc < 0) {
        LOG_ERR("fs_read failed: %d.", rc);
        return false;
    }

    out_len = rc;

    return true;
}

bool FsService::CreateDirectory(const std::string& relative_path) {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return false;
    }

    std::istringstream stream(relative_path);
    std::string segment;
    std::filesystem::path full_path(mountpoint_.mnt_point);

    while(std::getline(stream, segment, '/')) {
        if(segment.empty()) continue;

        // Append segment to path
        full_path /= segment;

        int rc = fs_mkdir(full_path.string().c_str());
        if(rc < 0 && rc != -EEXIST) {
            LOG_ERR("Failed to create dir '%s': %d.", full_path.string().c_str(), rc);
            return false;
        }
    }

    return true;
}

bool FsService::Exists(const std::string& relative_path) {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return false;
    }

    std::filesystem::path full_path(mountpoint_.mnt_point);
    full_path /= relative_path;

    struct fs_dirent entry;
    int rc = fs_stat(full_path.string().c_str(), &entry);

    return rc == 0;
}

bool FsService::DeleteFile(const std::string& relative_path) {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return false;
    }

    std::filesystem::path full_path(mountpoint_.mnt_point);
    full_path /= relative_path;

    int rc = fs_unlink(full_path.string().c_str());
    if(rc < 0) {
        LOG_ERR("fs_unlink failed: %d.", rc);
        return false;
    }

    return true;
}

bool FsService::DeleteRecursive(const std::string& relative_path) {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return false;
    }

    std::filesystem::path full_path(mountpoint_.mnt_point);
    full_path /= relative_path;

    struct fs_dir_t dir;
    struct fs_dirent entry;
    fs_dir_t_init(&dir);

    int rc = fs_opendir(&dir, full_path.string().c_str());
    if(rc < 0) {
        LOG_ERR("fs_opendir failed on path: %s (%d).", full_path.string().c_str(), rc);
        return false;
    }

    while(fs_readdir(&dir, &entry) == 0 && entry.name[0] != '\0') {
        std::filesystem::path entry_path(full_path);
        entry_path /= entry.name;

        if(entry.type == FS_DIR_ENTRY_FILE) {
            rc = fs_unlink(entry_path.string().c_str());
            if(rc < 0) {
                LOG_ERR("Failed to delete file: %s (%d).", entry_path.string().c_str(), rc);
                fs_closedir(&dir);
                return false;
            }
        } else if(entry.type == FS_DIR_ENTRY_DIR) {
            // Recurse into the directory
            if(!DeleteRecursive(entry_path.string())) {
                fs_closedir(&dir);
                return false;
            }

            // Remove the directory after its contents are gone
            rc = fs_unlink(entry_path.string().c_str());
            if(rc < 0) {
                LOG_ERR("Failed to delete dir: %s (%d).", entry_path.string().c_str(), rc);
                fs_closedir(&dir);
                return false;
            }
        }
    }

    fs_closedir(&dir);
    return true;
}


std::vector<std::string> FsService::ListFiles(const std::string& relative_path) const {
    std::vector<std::string> files;

    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return files;
    }

    std::filesystem::path full_path(mountpoint_.mnt_point);
    full_path /= relative_path;

    struct fs_dir_t dir;
    struct fs_dirent entry;
    fs_dir_t_init(&dir);

    if(fs_opendir(&dir, full_path.string().c_str()) < 0) {
        LOG_ERR("fs_opendir failed on path: %s.", full_path.string().c_str());
        return files;
    }

    while(fs_readdir(&dir, &entry) == 0 && entry.name[0] != '\0') {
        files.emplace_back(entry.name);
    }

    fs_closedir(&dir);

    return files;
}

size_t FsService::GetTotalSpace() const {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return 0;
    }

    struct fs_statvfs stat;
    if(fs_statvfs(mountpoint_.mnt_point, &stat) < 0) {
        LOG_ERR("fs_statvfs failed when querying total space.");
        return 0;
    }

    return stat.f_blocks * stat.f_bsize;
}

size_t FsService::GetUsedSpace() const {
    if(!IsMounted()) {
        LOG_ERR("Filesystem not mounted.");
        return 0;
    }

    struct fs_statvfs stat;
    if(fs_statvfs(mountpoint_.mnt_point, &stat) < 0) {
        LOG_ERR("fs_statvfs failed when querying used space.");
        return 0;
    }

    return (stat.f_blocks - stat.f_bfree) * stat.f_bsize;
}


bool FsService::Format() {
    if(IsMounted())
        Unmount();

    int rc = fs_mkfs(FS_LITTLEFS, (uintptr_t)mountpoint_.storage_dev, NULL, 0);

	if (rc < 0) {
		LOG_ERR("Format failed.");
		return false;
	}

    LOG_INF("Successfully formatted File System.");
    Mount();

    return true;
}

bool FsService::Mount() {
    if(IsMounted())
        return true;

    int rc = fs_mount(&mountpoint_);
    if(rc < 0)
        LOG_ERR("Failed to mount LittleFS: %d.", rc);

    if(rc == 0)
        LOG_INF("Mounted LittleFS at %s.", mountpoint_.mnt_point);
    else
        LOG_ERR("Failed to mount even after formatting.");

    return IsMounted();
}

void FsService::Unmount() {
    if(!IsMounted())
        return;

    int rc = fs_unmount(&mountpoint_);
    if(rc < 0)
        LOG_ERR("Failed to unmount LittleFS: %d.", rc);
    else
        LOG_INF("Unmounted LittleFS from %s.", mountpoint_.mnt_point);
}

bool FsService::IsMounted() const {
    return sys_dnode_is_linked(&mountpoint_.node);
}

} // namespace eerie_leap::subsys::fs::services

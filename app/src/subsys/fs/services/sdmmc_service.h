#pragma once

#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/sys/atomic.h>

#include "fs_service.h"

namespace eerie_leap::subsys::fs::services {

class SdmmcService : public FsService {
private:
    bool sd_card_present_ = false;
    k_sem sd_monitor_stop_sem_;
    atomic_t monitor_running_;

    static constexpr int k_stack_size_ = CONFIG_EERIE_LEAP_FS_SD_THREAD_STACK_SIZE;
    static constexpr int k_priority_ = K_PRIO_COOP(7);

    static z_thread_stack_element stack_area_[k_stack_size_];
    k_tid_t thread_id_;
    k_thread thread_data_;

    void SdMonitorThreadEntry();

    static bool IsSdCardAttached(const char* disk_name);
    void SdMonitorHandler();
    int SdMonitorStart();
    int SdMonitorStop();
    int PrintInfo() const;

public:
    SdmmcService(fs_mount_t mountpoint);

    bool Initialize() override;
};

} // namespace eerie_leap::subsys::fs::services

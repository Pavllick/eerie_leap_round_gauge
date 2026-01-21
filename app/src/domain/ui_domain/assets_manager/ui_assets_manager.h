#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <span>
#include <vector>

#include <zephyr/kernel.h>
#include <lvgl.h>

#include "utilities/memory/memory_resource_manager.h"
#include "subsys/fs/services/i_fs_service.h"

namespace eerie_leap::domain::ui_domain::assets_manager {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::subsys::fs::services;

class UiAssetsManager {
private:
    std::shared_ptr<IFsService> fs_service_;

    const std::string assets_dir_ = "ui_assets";

public:
    UiAssetsManager(std::shared_ptr<IFsService> fs_service);

    bool Save(std::string_view relative_path, std::span<const uint8_t> data);
    std::pmr::vector<uint8_t> Load(std::string_view relative_path);
    bool Delete(std::string_view relative_path);

    bool Exists(std::string_view relative_path) const;
};

} // namespace eerie_leap::domain::ui_domain::assets_manager

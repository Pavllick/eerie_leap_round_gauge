#include <filesystem>

#include "ui_assets_manager.h"

namespace eerie_leap::domain::ui_domain::assets_manager {

UiAssetsManager::UiAssetsManager(std::shared_ptr<IFsService> fs_service)
    : fs_service_(std::move(fs_service)) {

    if(!fs_service_->IsAvailable())
        return;

    if(!fs_service_->Exists(assets_dir_))
        fs_service_->CreateDirectory(assets_dir_);
}

bool UiAssetsManager::Save(std::string_view relative_path, std::span<const uint8_t> data) {
    if(!fs_service_->IsAvailable())
        return false;

    std::filesystem::path full_path(assets_dir_);
    full_path /= relative_path;

    return fs_service_->WriteFile(full_path.string(), data.data(), data.size(), false);
}

std::pmr::vector<uint8_t> UiAssetsManager::Load(std::string_view relative_path) {
    if(!fs_service_->IsAvailable())
        return {};

    std::filesystem::path full_path(assets_dir_);
    full_path /= relative_path;

    size_t file_size = fs_service_->GetFileSize(full_path.string());
    if(file_size == 0) {
        return {};
    }

    auto buffer = std::pmr::vector<uint8_t>(file_size, Mrm::GetExtPmr());
    size_t out_len = 0;

    if(!fs_service_->ReadFile(full_path.string(), buffer.data(), file_size, out_len) || out_len == 0) {
        return {};
    }

    return buffer;
}

bool UiAssetsManager::Delete(std::string_view relative_path) {
    if(!fs_service_->IsAvailable())
        return false;

    std::string full_path = assets_dir_ + "/" + std::string(relative_path);

    return fs_service_->DeleteFile(full_path);
}

bool UiAssetsManager::Exists(std::string_view relative_path) const {
    if(!fs_service_->IsAvailable())
        return false;

    std::filesystem::path full_path(assets_dir_);
    full_path /= relative_path;

    return fs_service_->Exists(full_path.string());
}

} // namespace eerie_leap::domain::ui_domain::assets_manager

#pragma once

#include <string>
#include <vector>

namespace eerie_leap::subsys::fs::services {

class IFsService {
public:
    virtual bool Initialize() = 0;
    virtual bool WriteFile(const std::string& relative_path, const void* data_p, size_t data_size) = 0;
    virtual bool ReadFile(const std::string& relative_path, void* data_p, size_t data_size, size_t& out_len) = 0;
    virtual bool CreateDirectory(const std::string& relative_path) = 0;
    virtual bool Exists(const std::string& relative_path) = 0;
    virtual bool DeleteFile(const std::string& relative_path) = 0;
    virtual bool DeleteRecursive(const std::string& relative_path = "") = 0;
    virtual std::vector<std::string> ListFiles(const std::string& relative_path = "") const = 0;
    virtual size_t GetTotalSpace() const = 0;
    virtual size_t GetUsedSpace() const = 0;
    virtual bool Format() = 0;
};

} // namespace eerie_leap::subsys::fs::services

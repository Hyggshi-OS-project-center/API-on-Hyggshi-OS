#include <HAPI/core/event.hpp>
#include <HAPI/core/permissions.hpp>
#include <HAPI/system/file_api.hpp>

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace hyggshios::system::file {

core::Result<std::string> read_text(const std::string& path) {
    if (!core::has_permission(core::Permission::FileRead)) {
        return core::Result<std::string>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileRead is required",
            0,
        });
    }

    std::ifstream ifs(path, std::ios::in);
    if (!ifs.is_open()) {
        const int err = errno;
        return core::Result<std::string>::failure({
            err == EACCES ? core::ErrorCode::PermissionDenied : core::ErrorCode::NotFound,
            "Failed to open file for reading: " + path + " (" + std::strerror(err) + ")",
            err,
        });
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return core::Result<std::string>::success(oss.str());
}

core::Result<std::vector<std::uint8_t>> read_bytes(const std::string& path) {
    if (!core::has_permission(core::Permission::FileRead)) {
        return core::Result<std::vector<std::uint8_t>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileRead is required",
            0,
        });
    }

    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        const int err = errno;
        return core::Result<std::vector<std::uint8_t>>::failure({
            core::ErrorCode::NotFound,
            "Failed to open binary file: " + path,
            err,
        });
    }
    std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(ifs)),
                                   std::istreambuf_iterator<char>());
    return core::Result<std::vector<std::uint8_t>>::success(std::move(data));
}

core::Result<bool> write_text(const std::string& path, const std::string& content) {
    if (!core::has_permission(core::Permission::FileWrite)) {
        return core::Result<bool>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileWrite is required",
            0,
        });
    }

    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        const int err = errno;
        return core::Result<bool>::failure({
            core::ErrorCode::IoError,
            "Failed to open file for writing: " + path + " (" + std::strerror(err) + ")",
            err,
        });
    }
    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (ofs.fail()) {
        return core::Result<bool>::failure({
            core::ErrorCode::IoError,
            "Failed while writing file: " + path,
            errno,
        });
    }
    core::EventBus::global().emit(core::Event{"file.written", "HAPI::file", path, 0});
    return core::Result<bool>::success(true);
}

core::Result<bool> write_bytes(const std::string& path, const std::vector<std::uint8_t>& data) {
    if (!core::has_permission(core::Permission::FileWrite)) {
        return core::Result<bool>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileWrite is required",
            0,
        });
    }

    std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        return core::Result<bool>::failure({
            core::ErrorCode::IoError,
            "Failed to open binary file for writing: " + path,
            errno,
        });
    }
    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (ofs.fail()) {
        return core::Result<bool>::failure({
            core::ErrorCode::IoError,
            "Failed while writing binary file: " + path,
            errno,
        });
    }
    core::EventBus::global().emit(core::Event{"file.written", "HAPI::file", path, 0});
    return core::Result<bool>::success(true);
}

core::Result<bool> copy(const std::string& src_path, const std::string& dst_path, bool overwrite) {
    if (!core::has_permission(core::Permission::FileRead) ||
        !core::has_permission(core::Permission::FileWrite)) {
        return core::Result<bool>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileRead and FileWrite are required",
            0,
        });
    }

    std::error_code ec;
    const auto opts = overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none;
    if (!fs::copy_file(src_path, dst_path, opts, ec)) {
        return core::Result<bool>::failure({
            core::ErrorCode::IoError,
            "Failed to copy file: " + ec.message(),
            ec.value(),
        });
    }
    return core::Result<bool>::success(true);
}

core::Result<bool> remove(const std::string& path) {
    if (!core::has_permission(core::Permission::FileWrite)) {
        return core::Result<bool>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileWrite is required",
            0,
        });
    }

    std::error_code ec;
    const bool removed = fs::remove(path, ec);
    if (ec) {
        return core::Result<bool>::failure({
            core::ErrorCode::IoError,
            "Failed to remove file: " + ec.message(),
            ec.value(),
        });
    }
    if (!removed) {
        return core::Result<bool>::failure({
            core::ErrorCode::NotFound,
            "File not found: " + path,
            ENOENT,
        });
    }
    core::EventBus::global().emit(core::Event{"file.deleted", "HAPI::file", path, 0});
    return core::Result<bool>::success(true);
}

bool exists(const std::string& path) {
    std::error_code ec;
    return fs::exists(path, ec);
}

core::Result<FileStat> stat(const std::string& path) {
    if (!core::has_permission(core::Permission::FileRead)) {
        return core::Result<FileStat>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileRead is required",
            0,
        });
    }

    std::error_code ec;
    const fs::file_status st = fs::symlink_status(path, ec);
    if (ec || !fs::exists(st)) {
        return core::Result<FileStat>::failure({
            core::ErrorCode::NotFound,
            "File not found: " + path,
            ec.value(),
        });
    }

    FileStat out;
    out.is_directory = fs::is_directory(st);
    out.is_regular_file = fs::is_regular_file(st);
    out.is_symlink = fs::is_symlink(st);
    if (out.is_regular_file) {
        out.size_bytes = fs::file_size(path, ec);
    }
    const auto ftime = fs::last_write_time(path, ec);
    if (!ec) {
        out.modified_unix = static_cast<std::int64_t>(ftime.time_since_epoch().count());
    }
    return core::Result<FileStat>::success(out);
}

core::Result<std::uint32_t> watch_once(const std::string& path,
                                       std::uint64_t last_mtime_ns,
                                       WatchCallback callback) {
    if (!core::has_permission(core::Permission::FileRead)) {
        return core::Result<std::uint32_t>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: FileRead is required",
            0,
        });
    }

    std::error_code ec;
    if (!fs::exists(path, ec)) {
        if (last_mtime_ns != 0) {
            if (callback) {
                callback(path, WatchEventKind::Deleted);
            }
            core::EventBus::global().emit(core::Event{"file.changed", "HAPI::file", path, 0});
            return core::Result<std::uint32_t>::success(1);
        }
        return core::Result<std::uint32_t>::success(0);
    }

    const auto ftime = fs::last_write_time(path, ec);
    const auto current_ns = static_cast<std::uint64_t>(ftime.time_since_epoch().count());
    if (current_ns != last_mtime_ns) {
        const auto kind = last_mtime_ns == 0 ? WatchEventKind::Created : WatchEventKind::Modified;
        if (callback) {
            callback(path, kind);
        }
        core::EventBus::global().emit(core::Event{"file.changed", "HAPI::file", path, 0});
        return core::Result<std::uint32_t>::success(1);
    }
    return core::Result<std::uint32_t>::success(0);
}

}  // namespace hyggshios::system::file

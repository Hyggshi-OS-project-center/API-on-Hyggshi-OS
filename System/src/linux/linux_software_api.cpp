#include <HAPI/core/permissions.hpp>
#include <HAPI/system/software_api.hpp>

#include <array>
#include <filesystem>
#include <set>
#include <sstream>
#include <string>

#include "linux_common.hpp"

namespace fs = std::filesystem;

namespace hyggshios::system::software {

std::vector<InstalledPackage> parse_dpkg_status(std::string_view content, std::size_t max_count) {
    std::vector<InstalledPackage> packages;
    InstalledPackage current;
    bool in_entry = false;

    auto flush = [&]() {
        if (in_entry && !current.name.empty()) {
            packages.push_back(std::move(current));
            current = InstalledPackage{};
        }
        in_entry = false;
    };

    std::size_t start = 0;
    while (start <= content.size()) {
        const std::size_t end = content.find('\n', start);
        const std::string_view line =
            detail::trim_ws(end == std::string_view::npos ? content.substr(start)
                                                          : content.substr(start, end - start));

        if (line.empty()) {
            flush();
            if (max_count > 0 && packages.size() >= max_count) {
                break;
            }
        } else {
            const std::size_t colon = line.find(':');
            if (colon != std::string_view::npos) {
                const std::string_view key = detail::trim_ws(line.substr(0, colon));
                const std::string_view val = detail::trim_ws(line.substr(colon + 1));
                in_entry = true;
                if (key == "Package") {
                    current.name = std::string(val);
                } else if (key == "Version") {
                    current.version = std::string(val);
                } else if (key == "Architecture") {
                    current.architecture = std::string(val);
                } else if (key == "Status") {
                    current.status = std::string(val);
                }
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    if (max_count == 0 || packages.size() < max_count) {
        flush();
    }

    return packages;
}

core::Result<std::vector<InstalledPackage>> try_list_installed_packages(std::size_t max_count) {
    if (!core::has_permission(core::Permission::SoftwareRead)) {
        return core::Result<std::vector<InstalledPackage>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SoftwareRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/var/lib/dpkg/status", content, err_no)) {
        return core::Result<std::vector<InstalledPackage>>::success({});
    }

    return core::Result<std::vector<InstalledPackage>>::success(
        parse_dpkg_status(content, max_count));
}

PackageManagerInfo get_package_manager_info() {
    static constexpr std::array<std::pair<const char*, const char*>, 6> kManagers = {{
        {"apt", "/usr/bin/apt"},
        {"dpkg", "/usr/bin/dpkg"},
        {"pacman", "/usr/bin/pacman"},
        {"dnf", "/usr/bin/dnf"},
        {"flatpak", "/usr/bin/flatpak"},
        {"snap", "/usr/bin/snap"},
    }};

    PackageManagerInfo info;
    std::error_code ec;
    for (const auto& [name, path] : kManagers) {
        if (fs::exists(path, ec)) {
            if (info.primary.empty()) {
                info.primary = name;
            }
            info.available.emplace_back(name);
        }
    }
    if (info.primary.empty()) {
        info.primary = "unknown";
    }
    return info;
}

core::Result<std::vector<SharedLibraryInfo>> try_list_loaded_libraries() {
    if (!core::has_permission(core::Permission::SoftwareRead)) {
        return core::Result<std::vector<SharedLibraryInfo>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SoftwareRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/self/maps", content, err_no)) {
        return core::Result<std::vector<SharedLibraryInfo>>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/self/maps",
            err_no,
        });
    }

    std::set<std::string> unique_paths;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        const std::size_t slash = line.find('/');
        if (slash == std::string::npos) {
            continue;
        }
        const std::string path = std::string(detail::trim_ws(std::string_view(line).substr(slash)));
        if (path.find(".so") != std::string::npos) {
            unique_paths.insert(path);
        }
    }

    std::vector<SharedLibraryInfo> libs;
    libs.reserve(unique_paths.size());
    for (const auto& p : unique_paths) {
        libs.push_back(SharedLibraryInfo{p});
    }
    return core::Result<std::vector<SharedLibraryInfo>>::success(std::move(libs));
}

}  // namespace hyggshios::system::software

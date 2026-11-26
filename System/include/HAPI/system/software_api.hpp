#pragma once

#include <HAPI/core/result.hpp>

#include <string>
#include <vector>

namespace hyggshios::system::software {

/// Installed system package entry (e.g., from /var/lib/dpkg/status).
struct InstalledPackage {
    std::string name;
    std::string version;
    std::string architecture;
    std::string status;
};

/// Detected system package managers.
struct PackageManagerInfo {
    std::string primary;                 ///< e.g., "apt", "dpkg", "pacman", "dnf", "unknown"
    std::vector<std::string> available;  ///< All detected package manager binaries
};

/// Loaded shared library entry from /proc/self/maps.
struct SharedLibraryInfo {
    std::string path;
};

/// Parses dpkg status formatted text into InstalledPackage entries.
[[nodiscard]] std::vector<InstalledPackage> parse_dpkg_status(std::string_view content,
                                                              std::size_t max_count = 0);

[[nodiscard]] core::Result<std::vector<InstalledPackage>> try_list_installed_packages(
    std::size_t max_count = 200);
[[nodiscard]] PackageManagerInfo get_package_manager_info();
[[nodiscard]] core::Result<std::vector<SharedLibraryInfo>> try_list_loaded_libraries();

}  // namespace hyggshios::system::software

namespace hyggshi = hyggshios;

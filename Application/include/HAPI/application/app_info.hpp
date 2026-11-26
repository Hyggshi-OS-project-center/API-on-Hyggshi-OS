#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace hyggshios::application {

/// Metadata for the currently running application process.
struct AppInfo {
    std::int32_t pid{0};
    std::string name;
    std::string executable_path;
    std::string working_directory;
};

/// Installed desktop application entry parsed from .desktop files.
struct DesktopAppEntry {
    std::string id;
    std::string name;
    std::string exec;
    std::string icon;
    std::string categories;
};

[[nodiscard]] core::Result<AppInfo> try_get_current_app();
[[nodiscard]] core::Result<std::vector<DesktopAppEntry>> try_list_installed_apps(
    std::size_t max_count = 100);

}  // namespace hyggshios::application

namespace hyggshi = hyggshios;

#include <HAPI/application/app_info.hpp>
#include <HAPI/core/permissions.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <unistd.h>

namespace fs = std::filesystem;

namespace hyggshios::application {

namespace {

DesktopAppEntry parse_desktop_file(const fs::path& path) {
    DesktopAppEntry entry;
    entry.id = path.filename().string();

    std::ifstream ifs(path);
    std::string line;
    bool in_desktop_entry = false;
    while (std::getline(ifs, line)) {
        if (line == "[Desktop Entry]") {
            in_desktop_entry = true;
            continue;
        }
        if (!line.empty() && line.front() == '[') {
            if (in_desktop_entry) {
                break;
            }
            continue;
        }
        if (!in_desktop_entry) {
            continue;
        }
        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        if (key == "Name" && entry.name.empty()) {
            entry.name = val;
        } else if (key == "Exec" && entry.exec.empty()) {
            entry.exec = val;
        } else if (key == "Icon" && entry.icon.empty()) {
            entry.icon = val;
        } else if (key == "Categories" && entry.categories.empty()) {
            entry.categories = val;
        }
    }
    return entry;
}

}  // namespace

core::Result<AppInfo> try_get_current_app() {
    AppInfo info;
    info.pid = static_cast<std::int32_t>(::getpid());

    std::error_code ec;
    info.working_directory = fs::current_path(ec).string();

    char exe_buf[4096]{};
    const ssize_t len = ::readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
    if (len > 0) {
        exe_buf[len] = '\0';
        info.executable_path = exe_buf;
        info.name = fs::path(info.executable_path).filename().string();
    }

    return core::Result<AppInfo>::success(std::move(info));
}

core::Result<std::vector<DesktopAppEntry>> try_list_installed_apps(std::size_t max_count) {
    if (!core::has_permission(core::Permission::SoftwareRead)) {
        return core::Result<std::vector<DesktopAppEntry>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SoftwareRead is required",
            0,
        });
    }

    std::vector<fs::path> search_dirs = {"/usr/share/applications"};
    if (const char* home = std::getenv("HOME")) {
        search_dirs.push_back(fs::path(home) / ".local/share/applications");
    }

    std::vector<DesktopAppEntry> apps;
    std::error_code ec;
    for (const auto& dir : search_dirs) {
        if (!fs::exists(dir, ec)) {
            continue;
        }
        for (const auto& file : fs::directory_iterator(dir, ec)) {
            if (file.path().extension() != ".desktop") {
                continue;
            }
            auto entry = parse_desktop_file(file.path());
            if (!entry.name.empty()) {
                apps.push_back(std::move(entry));
                if (max_count > 0 && apps.size() >= max_count) {
                    return core::Result<std::vector<DesktopAppEntry>>::success(std::move(apps));
                }
            }
        }
    }

    return core::Result<std::vector<DesktopAppEntry>>::success(std::move(apps));
}

}  // namespace hyggshios::application

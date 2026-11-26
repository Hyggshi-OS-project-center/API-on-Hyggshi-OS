#include <HAPI/core/permissions.hpp>

#include <algorithm>
#include <cstring>
#include <string>

#include <dirent.h>

#include "linux_adapters.hpp"
#include "linux_common.hpp"

namespace hyggshios::system {

ProcessInfo parse_proc_status(std::int32_t pid, std::string_view status_content) {
    ProcessInfo info;
    info.pid = pid;

    std::size_t start = 0;
    while (start <= status_content.size()) {
        const std::size_t end = status_content.find('\n', start);
        const std::string_view line = detail::trim_ws(
            end == std::string_view::npos ? status_content.substr(start)
                                          : status_content.substr(start, end - start));

        const std::size_t colon = line.find(':');
        if (colon != std::string_view::npos) {
            const std::string_view key = detail::trim_ws(line.substr(0, colon));
            const std::string_view val = detail::trim_ws(line.substr(colon + 1));

            if (key == "Name") {
                info.name = std::string(val);
            } else if (key == "State") {
                if (!val.empty()) {
                    info.state = std::string(1, val.front());
                }
            } else if (key == "PPid") {
                detail::parse_int(val, info.ppid);
            } else if (key == "VmRSS") {
                detail::parse_leading_int(val, info.memory_rss_kb);
            } else if (key == "Threads") {
                detail::parse_int(val, info.threads);
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }

    return info;
}

namespace detail {

core::Result<ProcessInfo> query_linux_process_info(std::int32_t pid) {
    if (!core::has_permission(core::Permission::ProcessInfoRead)) {
        return core::Result<ProcessInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: ProcessInfoRead is required",
            0,
        });
    }

    if (pid <= 0) {
        return core::Result<ProcessInfo>::failure({
            core::ErrorCode::InvalidArgument,
            "PID must be a positive integer",
            0,
        });
    }

    const std::string status_path = "/proc/" + std::to_string(pid) + "/status";
    std::string content;
    int err_no = 0;
    if (!read_file_text(status_path.c_str(), content, err_no)) {
        return core::Result<ProcessInfo>::failure({
            core::ErrorCode::NotFound,
            "Failed to read " + status_path + ": " + std::string(std::strerror(err_no)),
            err_no,
        });
    }

    return core::Result<ProcessInfo>::success(parse_proc_status(pid, content));
}

core::Result<std::vector<ProcessInfo>> query_linux_process_list() {
    if (!core::has_permission(core::Permission::ProcessInfoRead)) {
        return core::Result<std::vector<ProcessInfo>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: ProcessInfoRead is required",
            0,
        });
    }

    DIR* dir = ::opendir("/proc");
    if (dir == nullptr) {
        const int err_no = errno;
        return core::Result<std::vector<ProcessInfo>>::failure({
            core::ErrorCode::IoError,
            "Failed to open /proc: " + std::string(std::strerror(err_no)),
            err_no,
        });
    }

    std::vector<ProcessInfo> processes;
    while (struct dirent* entry = ::readdir(dir)) {
        std::int32_t pid = 0;
        if (!parse_int(entry->d_name, pid) || pid <= 0) {
            continue;
        }
        auto proc_res = query_linux_process_info(pid);
        if (proc_res.ok()) {
            processes.push_back(std::move(proc_res).value());
        }
    }
    ::closedir(dir);

    std::ranges::sort(processes,
                      [](const ProcessInfo& a, const ProcessInfo& b) { return a.pid < b.pid; });

    return core::Result<std::vector<ProcessInfo>>::success(std::move(processes));
}

}  // namespace detail
}  // namespace hyggshios::system

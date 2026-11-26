#include <HAPI/core/permissions.hpp>

#include <cstring>
#include <string>

#include "linux_adapters.hpp"
#include "linux_common.hpp"

namespace hyggshios::system {

MemoryInfo parse_meminfo(std::string_view content) {
    MemoryInfo info;
    bool has_available = false;

    std::size_t start = 0;
    while (start <= content.size()) {
        const std::size_t end = content.find('\n', start);
        const std::string_view line =
            detail::trim_ws(end == std::string_view::npos ? content.substr(start)
                                                          : content.substr(start, end - start));

        const std::size_t colon = line.find(':');
        if (colon != std::string_view::npos) {
            const std::string_view key = detail::trim_ws(line.substr(0, colon));
            const std::string_view val = detail::trim_ws(line.substr(colon + 1));

            std::uint64_t kb = 0;
            if (detail::parse_leading_int(val, kb)) {
                if (key == "MemTotal") {
                    info.total_kb = kb;
                } else if (key == "MemFree") {
                    info.free_kb = kb;
                } else if (key == "MemAvailable") {
                    info.available_kb = kb;
                    has_available = true;
                } else if (key == "Buffers") {
                    info.buffers_kb = kb;
                } else if (key == "Cached") {
                    info.cached_kb = kb;
                } else if (key == "SwapTotal") {
                    info.swap_total_kb = kb;
                } else if (key == "SwapFree") {
                    info.swap_free_kb = kb;
                }
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }

    if (!has_available) {
        info.available_kb = info.free_kb;
    }

    info.used_kb = info.total_kb >= info.available_kb ? (info.total_kb - info.available_kb) : 0;
    info.swap_used_kb =
        info.swap_total_kb >= info.swap_free_kb ? (info.swap_total_kb - info.swap_free_kb) : 0;

    if (info.total_kb > 0) {
        info.usage_percent =
            (static_cast<double>(info.used_kb) / static_cast<double>(info.total_kb)) * 100.0;
    }

    return info;
}

namespace detail {

core::Result<MemoryInfo> query_linux_memory_info() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<MemoryInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!read_file_text("/proc/meminfo", content, err_no)) {
        return core::Result<MemoryInfo>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/meminfo: " + std::string(std::strerror(err_no)),
            err_no,
        });
    }

    return core::Result<MemoryInfo>::success(parse_meminfo(content));
}

}  // namespace detail
}  // namespace hyggshios::system

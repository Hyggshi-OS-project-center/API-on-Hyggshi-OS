#include <HAPI/core/permissions.hpp>

#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <utility>

#include <unistd.h>

#include "linux_adapters.hpp"
#include "linux_common.hpp"

namespace hyggshios::system {

CpuInfo parse_cpuinfo(std::string_view content) {
    CpuInfo info;
    std::set<std::pair<std::string, std::string>> unique_cores;
    std::string current_phys_id;
    std::string current_core_id;
    std::uint32_t reported_cpu_cores = 0;

    auto flush_core_ids = [&]() {
        if (!current_phys_id.empty() && !current_core_id.empty()) {
            unique_cores.emplace(current_phys_id, current_core_id);
        }
        current_phys_id.clear();
        current_core_id.clear();
    };

    std::size_t start = 0;
    while (start <= content.size()) {
        const std::size_t end = content.find('\n', start);
        const std::string_view line =
            detail::trim_ws(end == std::string_view::npos ? content.substr(start)
                                                          : content.substr(start, end - start));

        if (line.empty()) {
            flush_core_ids();
        } else {
            const std::size_t colon = line.find(':');
            if (colon != std::string_view::npos) {
                const std::string_view key = detail::trim_ws(line.substr(0, colon));
                const std::string_view val = detail::trim_ws(line.substr(colon + 1));

                if (key == "processor") {
                    ++info.logical_cores;
                } else if (key == "model name" && info.model_name.empty()) {
                    info.model_name = std::string(val);
                } else if (key == "vendor_id" && info.vendor_id.empty()) {
                    info.vendor_id = std::string(val);
                } else if (key == "cpu MHz" && info.frequency_mhz == 0.0) {
                    info.frequency_mhz = std::strtod(std::string(val).c_str(), nullptr);
                } else if (key == "cache size" && info.cache_size_kb == 0) {
                    std::uint64_t kb = 0;
                    if (detail::parse_leading_int(val, kb)) {
                        info.cache_size_kb = kb;
                    }
                } else if (key == "physical id") {
                    current_phys_id = std::string(val);
                } else if (key == "core id") {
                    current_core_id = std::string(val);
                } else if (key == "cpu cores" && reported_cpu_cores == 0) {
                    detail::parse_int(val, reported_cpu_cores);
                }
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    flush_core_ids();

    if (!unique_cores.empty()) {
        info.physical_cores = static_cast<std::uint32_t>(unique_cores.size());
    } else if (reported_cpu_cores > 0) {
        info.physical_cores = reported_cpu_cores;
    } else {
        info.physical_cores = info.logical_cores;
    }

    return info;
}

namespace detail {

core::Result<CpuInfo> query_linux_cpu_info() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<CpuInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!read_file_text("/proc/cpuinfo", content, err_no)) {
        return core::Result<CpuInfo>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/cpuinfo: " + std::string(std::strerror(err_no)),
            err_no,
        });
    }

    CpuInfo info = parse_cpuinfo(content);

    if (info.logical_cores == 0) {
        const long nprocs = ::sysconf(_SC_NPROCESSORS_ONLN);
        if (nprocs > 0) {
            info.logical_cores = static_cast<std::uint32_t>(nprocs);
            if (info.physical_cores == 0) {
                info.physical_cores = info.logical_cores;
            }
        }
    }

    // Fallback to cpufreq scaling_cur_freq (reported in kHz) if /proc/cpuinfo lacked cpu MHz
    if (info.frequency_mhz == 0.0) {
        std::string freq_text;
        int freq_err = 0;
        if (read_file_text(
                "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", freq_text, freq_err)) {
            std::uint64_t khz = 0;
            if (parse_int(freq_text, khz)) {
                info.frequency_mhz = static_cast<double>(khz) / 1000.0;
            }
        }
    }

    return core::Result<CpuInfo>::success(std::move(info));
}

}  // namespace detail
}  // namespace hyggshios::system

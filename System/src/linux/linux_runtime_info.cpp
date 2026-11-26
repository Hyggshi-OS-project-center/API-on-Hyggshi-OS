#include "linux_common.hpp"

#include <HAPI/core/permissions.hpp>
#include <HAPI/system/runtime_info.hpp>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unistd.h>

extern char** environ;

namespace hyggshios::system {

namespace {

std::string format_tm(const std::tm& tm_val) {
    char buf[32]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm_val);
    return std::string(buf);
}

std::string get_env_or_empty(const char* key) {
    const char* val = std::getenv(key);
    return val != nullptr ? std::string(val) : std::string{};
}

} // namespace

core::Result<UptimeInfo> try_get_uptime_info() {
    if (!core::has_permission(core::Permission::SystemInfoRead)) {
        return core::Result<UptimeInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SystemInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/uptime", content, err_no)) {
        return core::Result<UptimeInfo>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/uptime: " + std::string(std::strerror(err_no)),
            err_no,
        });
    }

    std::istringstream iss(content);
    UptimeInfo info;
    if (!(iss >> info.uptime_seconds >> info.idle_seconds)) {
        return core::Result<UptimeInfo>::failure({
            core::ErrorCode::ParseError,
            "Failed to parse /proc/uptime",
            0,
        });
    }
    return core::Result<UptimeInfo>::success(info);
}

UptimeInfo get_uptime_info() {
    auto res = try_get_uptime_info();
    return res.ok() ? res.value() : UptimeInfo{};
}

core::Result<TimeInfo> try_get_time_info() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm utc_tm{};
    std::tm local_tm{};
    ::gmtime_r(&now_c, &utc_tm);
    ::localtime_r(&now_c, &local_tm);

    TimeInfo info;
    info.unix_timestamp_sec = static_cast<std::int64_t>(now_c);
    info.iso8601_utc = format_tm(utc_tm) + "Z";
    info.iso8601_local = format_tm(local_tm);
    info.timezone = local_tm.tm_zone != nullptr ? std::string(local_tm.tm_zone) : std::string("UTC");
    info.utc_offset_seconds = static_cast<std::int32_t>(local_tm.tm_gmtoff);

    return core::Result<TimeInfo>::success(std::move(info));
}

TimeInfo get_time_info() {
    auto res = try_get_time_info();
    return res.ok() ? std::move(res).value() : TimeInfo{};
}

core::Result<LocaleInfo> try_get_locale_info() {
    LocaleInfo info;
    info.lang = get_env_or_empty("LANG");
    info.lc_all = get_env_or_empty("LC_ALL");
    info.lc_ctype = get_env_or_empty("LC_CTYPE");
    info.lc_time = get_env_or_empty("LC_TIME");
    return core::Result<LocaleInfo>::success(std::move(info));
}

LocaleInfo get_locale_info() {
    auto res = try_get_locale_info();
    return res.ok() ? std::move(res).value() : LocaleInfo{};
}

core::Result<BootInfo> try_get_boot_info() {
    if (!core::has_permission(core::Permission::SystemInfoRead)) {
        return core::Result<BootInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SystemInfoRead is required",
            0,
        });
    }

    BootInfo info;
    int err_no = 0;

    std::string cmdline;
    if (detail::read_file_text("/proc/cmdline", cmdline, err_no)) {
        info.kernel_cmdline = std::string(detail::trim_ws(cmdline));
    }

    std::string boot_id;
    if (detail::read_file_text("/proc/sys/kernel/random/boot_id", boot_id, err_no)) {
        info.boot_id = std::string(detail::trim_ws(boot_id));
    }

    std::string stat_text;
    if (detail::read_file_text("/proc/stat", stat_text, err_no)) {
        std::istringstream iss(stat_text);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.rfind("btime ", 0) == 0) {
                detail::parse_int(std::string_view(line).substr(6), info.boot_time_unix);
                break;
            }
        }
    }

    return core::Result<BootInfo>::success(std::move(info));
}

BootInfo get_boot_info() {
    auto res = try_get_boot_info();
    return res.ok() ? std::move(res).value() : BootInfo{};
}

std::string get_env(const std::string& name) {
    return get_env_or_empty(name.c_str());
}

std::unordered_map<std::string, std::string> list_env() {
    std::unordered_map<std::string, std::string> out;
    if (environ == nullptr) {
        return out;
    }
    for (char** env = environ; *env != nullptr; ++env) {
        const std::string_view entry(*env);
        const std::size_t eq = entry.find('=');
        if (eq != std::string_view::npos && eq > 0) {
            out.emplace(std::string(entry.substr(0, eq)), std::string(entry.substr(eq + 1)));
        }
    }
    return out;
}

} // namespace hyggshios::system

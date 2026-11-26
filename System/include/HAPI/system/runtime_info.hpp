#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace hyggshios::system {

/// System uptime and idle time from /proc/uptime.
struct UptimeInfo {
    double uptime_seconds{0.0};
    double idle_seconds{0.0};
};

/// System clock and timezone metadata.
struct TimeInfo {
    std::int64_t unix_timestamp_sec{0};
    std::string iso8601_utc;
    std::string iso8601_local;
    std::string timezone;
    std::int32_t utc_offset_seconds{0};
};

/// Active locale configuration.
struct LocaleInfo {
    std::string lang;
    std::string lc_all;
    std::string lc_ctype;
    std::string lc_time;
};

/// Kernel boot parameters and boot timestamp.
struct BootInfo {
    std::uint64_t boot_time_unix{0};
    std::string kernel_cmdline;
    std::string boot_id;
};

[[nodiscard]] core::Result<UptimeInfo> try_get_uptime_info();
[[nodiscard]] UptimeInfo get_uptime_info();

[[nodiscard]] core::Result<TimeInfo> try_get_time_info();
[[nodiscard]] TimeInfo get_time_info();

[[nodiscard]] core::Result<LocaleInfo> try_get_locale_info();
[[nodiscard]] LocaleInfo get_locale_info();

[[nodiscard]] core::Result<BootInfo> try_get_boot_info();
[[nodiscard]] BootInfo get_boot_info();

/// Reads a single environment variable (returns empty string if unset).
[[nodiscard]] std::string get_env(const std::string& name);

/// Returns all environment variables visible to the current process.
[[nodiscard]] std::unordered_map<std::string, std::string> list_env();

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

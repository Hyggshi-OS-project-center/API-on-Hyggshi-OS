#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hyggshios::system {

/// Metadata and resource usage for a running process.
struct ProcessInfo {
    std::int32_t pid{0};            ///< Process ID
    std::int32_t ppid{0};           ///< Parent process ID
    std::string name;               ///< Executable / process name
    std::string state;              ///< Single-letter or descriptive state (e.g., "R", "S")
    std::uint64_t memory_rss_kb{0}; ///< Resident Set Size (VmRSS) in KiB
    std::uint32_t threads{0};       ///< Number of threads in the process
};

/// Parses Linux /proc/<pid>/status formatted text into a ProcessInfo struct.
[[nodiscard]] ProcessInfo parse_proc_status(std::int32_t pid, std::string_view status_content);

/// Retrieves information for a specific process ID.
[[nodiscard]] core::Result<ProcessInfo> try_get_process_info(std::int32_t pid);

/// Retrieves information for the calling process.
[[nodiscard]] core::Result<ProcessInfo> try_get_current_process_info();

/// Lists active processes visible on the host.
[[nodiscard]] core::Result<std::vector<ProcessInfo>> try_list_processes();

/// Lists active processes visible on the host (best-effort on failure).
[[nodiscard]] std::vector<ProcessInfo> list_processes();

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

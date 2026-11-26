#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string_view>

namespace hyggshios::system {

/// Physical and swap memory statistics in kilobytes (KiB as reported by Linux /proc/meminfo).
struct MemoryInfo {
    std::uint64_t total_kb{0};      ///< Total usable physical RAM
    std::uint64_t free_kb{0};       ///< Completely unallocated physical RAM
    std::uint64_t available_kb{0};  ///< Estimated RAM available for starting new applications
    std::uint64_t used_kb{0};       ///< Used RAM (total_kb - available_kb)
    std::uint64_t buffers_kb{0};    ///< Kernel buffer cache
    std::uint64_t cached_kb{0};     ///< Page cache
    std::uint64_t swap_total_kb{0}; ///< Total swap space
    std::uint64_t swap_free_kb{0};  ///< Free swap space
    std::uint64_t swap_used_kb{0};  ///< Used swap space (swap_total_kb - swap_free_kb)
    double usage_percent{0.0};      ///< Physical memory utilization percentage [0.0, 100.0]
};

/// Parses Linux /proc/meminfo formatted text into a MemoryInfo struct.
[[nodiscard]] MemoryInfo parse_meminfo(std::string_view content);

/// Retrieves memory statistics for the current host.
[[nodiscard]] core::Result<MemoryInfo> try_get_memory_info();

/// Retrieves memory statistics for the current host (best-effort on failure).
[[nodiscard]] MemoryInfo get_memory_info();

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace hyggshios::system {

/// Processor and topology information for the host system.
struct CpuInfo {
    std::string model_name;        ///< CPU model string (e.g., "11th Gen Intel(R) Core(TM) i5-1135G7")
    std::string vendor_id;         ///< CPU vendor identifier (e.g., "GenuineIntel", "AuthenticAMD")
    std::uint32_t physical_cores{0}; ///< Number of physical CPU cores
    std::uint32_t logical_cores{0};  ///< Number of logical processors / hardware threads
    double frequency_mhz{0.0};     ///< Current or nominal CPU frequency in MHz
    std::uint64_t cache_size_kb{0};  ///< Reported CPU cache size in KiB/KB
};

/// Parses Linux /proc/cpuinfo formatted text into a CpuInfo struct.
[[nodiscard]] CpuInfo parse_cpuinfo(std::string_view content);

/// Retrieves CPU information for the current host.
[[nodiscard]] core::Result<CpuInfo> try_get_cpu_info();

/// Retrieves CPU information for the current host (best-effort on failure).
[[nodiscard]] CpuInfo get_cpu_info();

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

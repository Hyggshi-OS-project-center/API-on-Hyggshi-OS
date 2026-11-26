#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hyggshios::system {

/// Information about a single mounted filesystem or storage volume.
struct StorageDevice {
    std::string device;            ///< Backing device or source (e.g., "/dev/nvme0n1p2")
    std::string mount_point;       ///< Mount path (e.g., "/", "/home")
    std::string filesystem;        ///< Filesystem type (e.g., "ext4", "btrfs", "vfat")
    std::uint64_t total_bytes{0};  ///< Total capacity in bytes
    std::uint64_t free_bytes{0};   ///< Total free bytes (including reserved blocks)
    std::uint64_t available_bytes{0}; ///< Bytes available to non-privileged users
    std::uint64_t used_bytes{0};   ///< Used bytes (total_bytes - free_bytes)
    double usage_percent{0.0};     ///< Capacity usage percentage [0.0, 100.0]
};

/// Aggregated storage mount information for the host.
struct StorageInfo {
    std::vector<StorageDevice> mounts;
};

/// Parses /proc/mounts formatted text into a list of StorageDevice entries (without statvfs sizes).
/// When include_pseudo is false, virtual/pseudo filesystems (proc, sysfs, tmpfs, etc.) are filtered out.
[[nodiscard]] std::vector<StorageDevice> parse_proc_mounts(std::string_view content,
                                                           bool include_pseudo = false);

/// Retrieves mounted storage information for the current host.
[[nodiscard]] core::Result<StorageInfo> try_get_storage_info(bool include_pseudo = false);

/// Retrieves mounted storage information for the current host (best-effort on failure).
[[nodiscard]] StorageInfo get_storage_info(bool include_pseudo = false);

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

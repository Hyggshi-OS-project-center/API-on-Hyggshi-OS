#include <HAPI/core/permissions.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>
#include <string>

#include <sys/statvfs.h>

#include "linux_adapters.hpp"
#include "linux_common.hpp"

namespace hyggshios::system {

namespace {

bool is_pseudo_filesystem(std::string_view fs) noexcept {
    static constexpr std::array<std::string_view, 22> kPseudoFs = {
        "proc",        "sysfs",   "devtmpfs",  "devpts",   "tmpfs",   "cgroup",
        "cgroup2",     "pstore",  "bpf",       "tracefs",  "debugfs", "securityfs",
        "configfs",    "fusectl", "hugetlbfs", "mqueue",   "autofs",  "efivarfs",
        "binfmt_misc", "nsfs",    "ramfs",     "squashfs",
    };
    return std::ranges::any_of(kPseudoFs,
                               [fs](std::string_view candidate) { return fs == candidate; });
}

std::string unescape_mount_path(std::string_view raw) {
    std::string out;
    out.reserve(raw.size());
    for (std::size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 3 < raw.size()) {
            // /proc/mounts encodes spaces, tabs, newlines, and backslashes as \040, \011, \012, \134
            const std::string_view oct = raw.substr(i + 1, 3);
            if (oct == "040") {
                out.push_back(' ');
                i += 3;
                continue;
            }
            if (oct == "011") {
                out.push_back('\t');
                i += 3;
                continue;
            }
            if (oct == "012") {
                out.push_back('\n');
                i += 3;
                continue;
            }
            if (oct == "134") {
                out.push_back('\\');
                i += 3;
                continue;
            }
        }
        out.push_back(raw[i]);
    }
    return out;
}

}  // namespace

std::vector<StorageDevice> parse_proc_mounts(std::string_view content, bool include_pseudo) {
    std::vector<StorageDevice> mounts;

    std::size_t start = 0;
    while (start <= content.size()) {
        const std::size_t end = content.find('\n', start);
        const std::string_view line =
            detail::trim_ws(end == std::string_view::npos ? content.substr(start)
                                                          : content.substr(start, end - start));

        if (!line.empty()) {
            std::istringstream iss{std::string(line)};
            std::string dev;
            std::string mnt;
            std::string fs_type;
            if (iss >> dev >> mnt >> fs_type) {
                if (include_pseudo || !is_pseudo_filesystem(fs_type)) {
                    StorageDevice entry;
                    entry.device = std::move(dev);
                    entry.mount_point = unescape_mount_path(mnt);
                    entry.filesystem = std::move(fs_type);
                    mounts.push_back(std::move(entry));
                }
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }

    return mounts;
}

namespace detail {

core::Result<StorageInfo> query_linux_storage_info(bool include_pseudo) {
    if (!core::has_permission(core::Permission::StorageInfoRead)) {
        return core::Result<StorageInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: StorageInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!read_file_text("/proc/mounts", content, err_no)) {
        return core::Result<StorageInfo>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/mounts: " + std::string(std::strerror(err_no)),
            err_no,
        });
    }

    StorageInfo info;
    info.mounts = parse_proc_mounts(content, include_pseudo);

    for (auto& mount : info.mounts) {
        struct statvfs vfs{};
        if (::statvfs(mount.mount_point.c_str(), &vfs) == 0) {
            const std::uint64_t block_size = vfs.f_frsize > 0
                                                 ? static_cast<std::uint64_t>(vfs.f_frsize)
                                                 : static_cast<std::uint64_t>(vfs.f_bsize);
            mount.total_bytes = static_cast<std::uint64_t>(vfs.f_blocks) * block_size;
            mount.free_bytes = static_cast<std::uint64_t>(vfs.f_bfree) * block_size;
            mount.available_bytes = static_cast<std::uint64_t>(vfs.f_bavail) * block_size;
            mount.used_bytes =
                mount.total_bytes >= mount.free_bytes ? (mount.total_bytes - mount.free_bytes) : 0;
            if (mount.total_bytes > 0) {
                mount.usage_percent = (static_cast<double>(mount.used_bytes) /
                                       static_cast<double>(mount.total_bytes)) *
                                      100.0;
            }
        }
    }

    return core::Result<StorageInfo>::success(std::move(info));
}

}  // namespace detail
}  // namespace hyggshios::system

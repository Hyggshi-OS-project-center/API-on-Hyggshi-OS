#pragma once

#include <HAPI/core/result.hpp>

#include <string>
#include <string_view>

namespace hyggshios::system {

/// Operating system and kernel information for the host environment.
struct OsInfo {
    std::string name;            ///< OS Name (e.g., "Hyggshi OS", "Ubuntu")
    std::string version;         ///< OS Version (e.g., "2.0.0", "24.04")
    std::string id;              ///< OS ID (e.g., "hyggshi", "ubuntu")
    std::string codename;        ///< OS Codename (e.g., "pyramid", "noble")
    std::string pretty_name;     ///< Human-friendly OS description
    std::string kernel_name;     ///< Kernel name from uname() (e.g., "Linux")
    std::string kernel_version;  ///< Kernel release from uname() (e.g., "7.0.0-43-generic")
    std::string architecture;    ///< Machine architecture from uname() (e.g., "x86_64")
    std::string hostname;        ///< System hostname from gethostname()
};

/// Parses freedesktop.org os-release content into the OS metadata fields of OsInfo.
/// Kernel and hostname fields are left empty by this parser.
[[nodiscard]] OsInfo parse_os_release(std::string_view content);

/// Retrieves operating system information for the current host.
/// Returns a Result containing OsInfo on success or a structured Error on failure.
[[nodiscard]] core::Result<OsInfo> try_get_os_info();

/// Retrieves operating system information for the current host.
/// Returns best-effort OsInfo on partial failure.
[[nodiscard]] OsInfo get_os_info();

}  // namespace hyggshios::system

namespace hyggshi = hyggshios;

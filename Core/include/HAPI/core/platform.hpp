#pragma once

#include <string>

namespace hyggshios::core {

/// Supported host platform targets for Platform Adapters.
enum class PlatformType {
    Linux,
    Windows,
    Web,
    Unknown
};

/// Metadata describing the active platform adapter and build target.
struct PlatformInfo {
    PlatformType type{PlatformType::Unknown};
    std::string name;         ///< "linux", "windows", "web", or "unknown"
    std::string abi_version;  ///< Public C ABI version
    bool is_posix{false};
    bool supports_native_syscalls{false};
};

/// Returns compile-time / runtime platform adapter metadata.
[[nodiscard]] PlatformInfo get_platform_info();

} // namespace hyggshios::core

namespace hyggshi = hyggshios;

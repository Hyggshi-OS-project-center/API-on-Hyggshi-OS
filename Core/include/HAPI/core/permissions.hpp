#pragma once

#include <cstdint>

namespace hyggshios::core {

/// Capability flags for System, File, Network, Desktop, and Security operations.
enum class Permission : std::uint32_t {
    SystemInfoRead = 1U << 0U,
    HardwareInfoRead = 1U << 1U,
    ProcessInfoRead = 1U << 2U,
    StorageInfoRead = 1U << 3U,
    NetworkInfoRead = 1U << 4U,
    FileRead = 1U << 5U,
    FileWrite = 1U << 6U,
    SecurityRead = 1U << 7U,
    SoftwareRead = 1U << 8U,
    DesktopNotify = 1U << 9U,
};

/// Least-privilege security policy configuration.
/// By default, HAPI never auto-escalates privileges or grants root capabilities.
struct SecurityPolicy {
    bool allow_privilege_escalation{false};  ///< Always false by default
    bool allow_file_write{true};
    bool allow_process_signals{false};
    std::uint32_t granted_mask{0xFFFFFFFFU};
};

/// Returns the active process-wide SecurityPolicy (mutable for policy sandboxing).
inline SecurityPolicy& active_security_policy() noexcept {
    static SecurityPolicy policy{};
    return policy;
}

/// Checks whether the current execution context is granted the requested permission.
[[nodiscard]] inline bool has_permission(Permission perm) noexcept {
    const auto& policy = active_security_policy();
    if (perm == Permission::FileWrite && !policy.allow_file_write) {
        return false;
    }
    return (policy.granted_mask & static_cast<std::uint32_t>(perm)) != 0;
}

}  // namespace hyggshios::core

namespace hyggshi = hyggshios;

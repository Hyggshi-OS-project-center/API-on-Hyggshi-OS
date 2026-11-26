#pragma once

#include <cstdint>

namespace hyggshios::core {

/// Capability flags for System, File, Network, Desktop, and Security operations.
enum class Permission : std::uint32_t {
    SystemInfoRead = 1 << 0,
    HardwareInfoRead = 1 << 1,
    ProcessInfoRead = 1 << 2,
    StorageInfoRead = 1 << 3,
    NetworkInfoRead = 1 << 4,
    FileRead = 1 << 5,
    FileWrite = 1 << 6,
    SecurityRead = 1 << 7,
    SoftwareRead = 1 << 8,
    DesktopNotify = 1 << 9,
};

/// Least-privilege security policy configuration.
/// By default, HAPI never auto-escalates privileges or grants root capabilities.
struct SecurityPolicy {
    bool allow_privilege_escalation{false}; ///< Always false by default
    bool allow_file_write{true};
    bool allow_process_signals{false};
    std::uint32_t granted_mask{0xFFFFFFFFu};
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

} // namespace hyggshios::core

namespace hyggshi = hyggshios;

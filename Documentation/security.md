# Security Model

HAPI is designed around a **least-privilege, no-auto-escalation** security model. No HAPI API will ever request, acquire, or silently elevate to `root` privileges on behalf of a caller.

## Core Principles

| Principle | Detail |
| :--- | :--- |
| **No auto-escalation** | `SecurityPolicy.allow_privilege_escalation` is always `false` by default and is never set internally |
| **No root by default** | All APIs operate within the calling process's existing UID/EUID; nothing uses `setuid`, `sudo`, or `pkexec` |
| **Capability-gated reads** | Every API call checks `has_permission(Permission::*)` before accessing system resources |
| **Policy is inspectable** | Callers can read the active policy via `active_security_policy()` |
| **Explicit grant** | Sandbox environments can narrow the granted permission mask by clearing bits in `SecurityPolicy.granted_mask` |

## Permission Flags

```cpp
enum class Permission : uint32_t {
    SystemInfoRead   = 1 << 0,  // OS, kernel, uptime, boot, locale, time
    HardwareInfoRead = 1 << 1,  // CPU, memory, GPU, battery, sensors, DMI
    ProcessInfoRead  = 1 << 2,  // /proc/<pid>/status, process listing
    StorageInfoRead  = 1 << 3,  // mount points, statvfs
    NetworkInfoRead  = 1 << 4,  // interfaces, DNS, connections
    FileRead         = 1 << 5,  // read_text, read_bytes, stat, exists
    FileWrite        = 1 << 6,  // write_text, write_bytes, copy, remove
    SecurityRead     = 1 << 7,  // user, groups, capabilities, session
    SoftwareRead     = 1 << 8,  // installed packages, libraries, services
    DesktopNotify    = 1 << 9,  // send_notification
};
```

All 10 flags are granted by default (`granted_mask = 0xFFFFFFFF`). A sandbox or policy host narrows access by clearing bits:

```cpp
// Revoke file write and notification permission for an untrusted plugin
auto& policy = hyggshios::core::active_security_policy();
policy.allow_file_write = false;
policy.granted_mask &= ~static_cast<uint32_t>(hyggshios::core::Permission::DesktopNotify);
```

## SecurityPolicy Fields

```cpp
struct SecurityPolicy {
    bool allow_privilege_escalation{false}; // NEVER set to true by HAPI internally
    bool allow_file_write{true};
    bool allow_process_signals{false};      // Reserved for future process control APIs
    uint32_t granted_mask{0xFFFFFFFFu};
};
```

## C ABI Error Codes for Permission Failures

When a caller lacks a required permission, HAPI C ABI returns `HAPI_ERR_PERMISSION` (-3). The C++ API returns `Result<T>` with `ErrorCode::PermissionDenied`.

## Sensitive Data Handling

- **Passwords / credentials**: HAPI never reads `/etc/shadow`, PAM databases, or any credential store.
- **Kernel capabilities**: `try_get_capabilities()` reads `/proc/self/status` for the **calling process only**.
- **Environment variables**: `list_env()` reflects `environ` of the **calling process only**; it cannot read another process's environment without `ProcessInfoRead` and explicit PID access.

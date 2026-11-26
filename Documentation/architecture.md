# Architecture

**API-on-Hyggshi-OS** (`HAPI`) uses a layered **Core + Platform Adapters + Public C ABI + Language Bindings** architecture so applications and services can query OS capabilities without handling raw platform files or syscalls directly.

## Directory Layout

| Component | Role |
| :--- | :--- |
| `Core/` | Common API logic, `Error`, `Result<T>`, `SecurityPolicy`, `Permission`, `EventBus`, and `PlatformInfo` |
| `System/` | System, Runtime, Hardware, Desktop, File, Storage, Network, Software, Process, and Security APIs |
| `Application/` | Current process/app metadata, `.desktop` app discovery, and desktop notifications |
| `Services/` | Systemd service status and listing APIs |
| `Bindings/` | Public C ABI (`libhapi_c.so`) and Python, LuaJIT, and TypeScript SDKs |
| `Examples/` | Runnable C++ usage examples |
| `Documentation/` | Architecture, roadmap, bindings guide, and API reference |
| `Tests/` | Unit and integration tests (`ctest`) |

## Layered Model

```text
+-------------------------------------------------------------------+
|                        Language Bindings                          |
|     C++20 SDK  |  C ABI (libhapi_c)  |  Python  |  Lua  |  TS     |
+-------------------------------------------------------------------+
                                 |
                                 v
+-------------------------------------------------------------------+
|                   Domain APIs (HAPI::HAPI)                        |
|        System  |  Application  |  Services  |  File/Network       |
+-------------------------------------------------------------------+
                                 |
                                 v
+-------------------------------------------------------------------+
|                         Core (HAPI::core)                         |
|      Result<T>  |  Error  |  SecurityPolicy  |  EventBus          |
+-------------------------------------------------------------------+
                                 |
          +----------------------+----------------------+
          |                      |                      |
          v                      v                      v
+-------------------+  +-------------------+  +---------------------+
|   Linux Adapter   |  |  Windows Adapter  |  |     Web Adapter     |
|  (/proc, /sys,    |  |     (Planned)     |  |      (Planned)      |
|   POSIX, D-Bus)   |  |                   |  |                     |
+-------------------+  +-------------------+  +---------------------+
```

## Namespaces & Targets

- Primary C++ namespace: `hyggshios` (`hyggshios::core`, `hyggshios::system`, `hyggshios::application`, `hyggshios::services`), with alias `namespace hyggshi = hyggshios;`.
- CMake targets: `HAPI::core`, `HAPI::system`, `HAPI::application`, `HAPI::services`, umbrella `HAPI::HAPI`, and shared C ABI `hapi_c`.
- Security model: `SecurityPolicy` defaults to least-privilege (`allow_privilege_escalation = false`) and never auto-escalates to root.

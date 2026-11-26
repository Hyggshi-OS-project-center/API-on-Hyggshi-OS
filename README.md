# API-on-Hyggshi-OS (HAPI)

**API-on-Hyggshi-OS** is the official system and platform API layer for Hyggshi OS — a clean C++20 SDK backed by a stable **public C ABI** and language bindings for **Python**, **LuaJIT**, and **TypeScript/Node.js**.

## Architecture

| Component | Role |
| :--- | :--- |
| `Core/` | Common API logic, `Error`, `Result<T>`, `SecurityPolicy`, `Permission`, `EventBus`, and `PlatformInfo` |
| `System/` | System, Runtime, Hardware, Desktop, File, Storage, Network, Software, Process, and Security APIs |
| `Application/` | Application metadata, `.desktop` app discovery, and desktop notifications |
| `Services/` | Systemd service status and listing |
| `Bindings/` | Public C ABI (`libhapi_c.so`) plus Python, LuaJIT, and TypeScript/Node.js SDKs |
| `Examples/` | Runnable usage examples (C++, C) |
| `Documentation/` | Architecture, roadmap, security model, bindings guide, and API reference |
| `Tests/` | Unit and integration tests (`ctest`) |

```
Language Bindings: C++20 SDK | C ABI | Python | LuaJIT | TypeScript/Node.js
                        |
                   Domain APIs
           System · Application · Services
                        |
                      Core
           Result<T> · Error · EventBus · SecurityPolicy
                        |
              Linux Adapter  |  Windows (Planned)  |  Web (Planned)
```

## Build & Test

Requires CMake ≥ 3.20 and a C++20 / C compiler (GCC 11+ or Clang 13+).

```bash
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected:

```
100% tests passed, 0 tests failed out of 3
```

## Quick Start — C++

```cpp
#include <HAPI/system/system.hpp>
#include <iostream>

int main() {
    auto os  = hyggshi::system::get_os_info();
    auto cpu = hyggshi::system::get_cpu_info();
    auto mem = hyggshi::system::get_memory_info();

    std::cout << os.name << ' ' << os.version << '\n';
    std::cout << cpu.physical_cores << " cores / " << cpu.logical_cores << " threads\n";
    std::cout << (mem.used_kb / 1024) << " / " << (mem.total_kb / 1024) << " MiB\n";
}
```

## Quick Start — Python

```python
from hapi import HapiClient
client = HapiClient()       # auto-discovers build/libhapi_c.so
os_info = client.get_os_info()
print(os_info.name, os_info.version)
```

```bash
python3 Bindings/python/test_hapi.py
```

## Quick Start — LuaJIT

```lua
local hapi = require("hapi")
local os_info = hapi.get_os_info()
print(os_info.name, os_info.version)
```

```bash
luajit Bindings/lua/test_hapi.lua
```

## Quick Start — TypeScript / Node.js

```js
import { createHapiClient } from "./Bindings/typescript/src/index.js";
const client = createHapiClient();
const os = await client.getOsInfo();
console.log(os.name, os.version);
```

```bash
node Bindings/typescript/test_hapi_node.mjs
```

## Quick Start — C

```c
#include <hapi_c.h>
#include <stdio.h>
int main(void) {
    hapi_os_info_t os = {0};
    if (hapi_get_os_info(&os) == HAPI_OK)
        printf("%s %s\n", os.name, os.version);
}
```

```bash
./build/c_sdk_example
```

## Domain Coverage

| Domain | Key APIs |
| :--- | :--- |
| **System OS/Runtime** | `get_os_info`, `get_uptime_info`, `get_time_info`, `get_locale_info`, `get_boot_info`, `get_env` |
| **Hardware** | `get_cpu_info`, `get_memory_info`, `get_hardware_info`, `try_get_gpu_devices`, `try_get_batteries`, `try_get_sensor_readings`, USB, PCI, Audio, Display |
| **Storage** | `get_storage_info`, `try_get_storage_info(include_pseudo)` |
| **Process** | `try_get_process_info`, `try_get_current_process_info`, `try_list_processes` |
| **Desktop** | `get_desktop_info`, `try_get_desktop_settings` |
| **File** | `read_text`, `write_text`, `read_bytes`, `write_bytes`, `copy`, `remove`, `stat`, `watch_once` |
| **Network** | `try_get_interfaces`, `try_get_dns_config`, `try_get_connections`, `try_get_wifi_info`, `get_proxy_config`, `try_get_statistics` |
| **Software** | `try_list_installed_packages`, `get_package_manager_info`, `try_list_loaded_libraries` |
| **Services** | `try_list_services`, `try_get_service_status` |
| **Security** | `try_get_current_user`, `try_get_current_groups`, `try_get_capabilities`, `try_get_session_info`, `get_security_policy` |
| **Application** | `try_get_current_app`, `try_list_installed_apps`, `send_notification` |
| **Core** | `EventBus::global()`, `has_permission`, `get_platform_info`, `Result<T>`, `Error` |

## Documentation

| Document | Description |
| :--- | :--- |
| [Getting Started](Documentation/getting-started.md) | Build, install, and quick-start for all SDKs |
| [Architecture](Documentation/architecture.md) | Layered model, namespaces, CMake targets |
| [Security Model](Documentation/security.md) | Least-privilege policy, permission flags, sensitive data handling |
| [Roadmap](Documentation/roadmap.md) | Implemented milestones and planned work |
| [Bindings Guide](Documentation/bindings.md) | C ABI, Python, LuaJIT, TypeScript SDK usage |
| [System & Hardware API](Documentation/api/system_and_hardware.md) | Reference for System, Runtime, and Hardware APIs |
| [File, Network, Software, Desktop & Security API](Documentation/api/file_network_security.md) | Reference for File, Network, Software, Desktop, and Security APIs |
| [Core, Application & Services API](Documentation/api/core_app_services.md) | Reference for Core, Application, and Services APIs |

## Linux Roadmap

| Milestone | API | Status |
| :--- | :--- | :--- |
| 01 | OS Information | ✅ Implemented |
| 02 | CPU Information | ✅ Implemented |
| 03 | Memory Information | ✅ Implemented |
| 04 | Storage Information | ✅ Implemented |
| 05 | Process Information | ✅ Implemented |
| 06 | Desktop Environment Information | ✅ Implemented |
| 07 | Hardware Information (DMI, GPU, Battery, Sensors…) | ✅ Implemented |
| 08 | Runtime (Uptime, Time, Locale, Boot, Env) | ✅ Implemented |
| 09 | File API | ✅ Implemented |
| 10 | Network API | ✅ Implemented |
| 11 | Software & Services | ✅ Implemented |
| 12 | Security & Permissions | ✅ Implemented |
| 13 | EventBus, Application & Notifications | ✅ Implemented |

## License

Released under the [MIT License](LICENSE).

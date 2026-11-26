# Getting Started with HAPI

**API-on-Hyggshi-OS** (HAPI) is the official system and platform API layer for Hyggshi OS. It exposes structured access to OS, hardware, desktop, file, network, security, and application information through a clean C++20 SDK backed by a stable **public C ABI** (`libhapi_c.so`) and language bindings for **Python**, **LuaJIT**, and **TypeScript**.

## Prerequisites

| Tool | Minimum version |
| :--- | :--- |
| CMake | 3.20 |
| GCC or Clang | C++20 support (GCC 11+, Clang 13+) |
| Ninja (optional) | any |
| Python 3 | 3.9+ (for Python SDK) |
| LuaJIT | 2.1+ (for Lua SDK) |
| Node.js | 18+ (for TypeScript SDK) |

## Clone & Build

```bash
git clone https://github.com/nguyenminhphucHSI/API-on-Hyggshi-OS.git
cd API-on-Hyggshi-OS

# Configure
cmake -B build -G Ninja          # or omit -G for make

# Build everything: libraries, C ABI, examples, tests
cmake --build build
```

After the build, the following artifacts are produced:

| Artifact | Description |
| :--- | :--- |
| `build/libhapi_core.a` | Core library (errors, results, events, permissions) |
| `build/libhapi_system.a` | System library (all platform adapters) |
| `build/libhapi_application.a` | Application and notification library |
| `build/libhapi_services.a` | Services library |
| `build/libhapi_c.so` | Public C ABI shared library (used by Python, Lua, TypeScript) |
| `build/os_info_example` | Simple OS info example |
| `build/system_overview_example` | Full system overview example |
| `build/hapi_tests` | OS info unit tests |
| `build/hapi_system_tests` | System info unit tests |
| `build/hapi_full_tests` | Full API + C ABI integration tests |

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

Expected output:

```
1/3 Test #1: HapiOsInfoTests ..................   Passed
2/3 Test #2: HapiSystemInfoTests ..............   Passed
3/3 Test #3: HapiFullApiTests .................   Passed

100% tests passed, 0 tests failed out of 3
```

## C++20 Quick Start

```cpp
#include <HAPI/system/system.hpp>
#include <HAPI/core/event.hpp>
#include <iostream>

int main() {
    // Subscribe to any file events emitted by the File API
    hyggshios::core::EventBus::global().subscribe("file.changed", [](const auto& ev) {
        std::cout << "File changed: " << ev.payload << '\n';
    });

    // OS & kernel info
    auto os = hyggshi::system::get_os_info();
    std::cout << os.name << ' ' << os.kernel_version << '\n';

    // CPU
    auto cpu = hyggshi::system::get_cpu_info();
    std::cout << cpu.physical_cores << " cores / " << cpu.logical_cores << " threads\n";

    // Memory
    auto mem = hyggshi::system::get_memory_info();
    std::cout << (mem.used_kb / 1024) << " / " << (mem.total_kb / 1024) << " MiB used\n";

    // Network interfaces
    auto ifaces = hyggshi::system::network::try_get_interfaces();
    if (ifaces.ok()) {
        for (const auto& iface : ifaces.value()) {
            if (!iface.is_loopback && iface.is_up) {
                std::cout << iface.name;
                for (const auto& ip : iface.ipv4_addresses) {
                    std::cout << " " << ip;
                }
                std::cout << '\n';
            }
        }
    }

    return 0;
}
```

## Python Quick Start

```python
from hapi import HapiClient

client = HapiClient()   # auto-discovers build/libhapi_c.so
os_info = client.get_os_info()
cpu     = client.get_cpu_info()
mem     = client.get_memory_info()

print(f"{os_info.name} {os_info.version}")
print(f"CPU: {cpu.model_name} ({cpu.logical_cores} threads)")
print(f"RAM: {mem.used_kb // 1024} / {mem.total_kb // 1024} MiB ({mem.usage_percent:.1f}%)")
```

```bash
python3 Bindings/python/test_hapi.py
```

## LuaJIT Quick Start

```lua
local hapi = require("hapi")

local os_info = hapi.get_os_info()
local cpu     = hapi.get_cpu_info()
local mem     = hapi.get_memory_info()

print(os_info.name, os_info.version)
print(cpu.model_name, cpu.logical_cores .. " threads")
print(string.format("%.1f%% RAM used", mem.usage_percent))
```

```bash
luajit Bindings/lua/test_hapi.lua
```

## C Quick Start

```c
#include <hapi_c.h>
#include <stdio.h>

int main(void) {
    hapi_os_info_t os = {0};
    if (hapi_get_os_info(&os) == HAPI_OK) {
        printf("OS: %s %s\n", os.name, os.version);
        printf("Kernel: %s %s [%s]\n", os.kernel_name, os.kernel_version, os.architecture);
    }

    hapi_cpu_info_t cpu = {0};
    if (hapi_get_cpu_info(&cpu) == HAPI_OK) {
        printf("CPU: %s (%u physical / %u logical)\n",
               cpu.model_name, cpu.physical_cores, cpu.logical_cores);
    }

    hapi_memory_info_t mem = {0};
    if (hapi_get_memory_info(&mem) == HAPI_OK) {
        printf("RAM: %llu / %llu MiB (%.1f%%)\n",
               mem.used_kb / 1024, mem.total_kb / 1024, mem.usage_percent);
    }
    return 0;
}
```

```bash
gcc -o c_example examples/c_example.c -I Bindings/c/include -L build -l hapi_c -Wl,-rpath,build
./c_example
```

## Error Handling

HAPI uses `Result<T>` throughout C++ to avoid exceptions on expected failures:

```cpp
auto result = hyggshi::system::try_get_cpu_info();
if (!result.ok()) {
    // result.error().code  — hyggshios::core::ErrorCode enum
    // result.error().message — human-readable string
    // result.error().os_errno — raw errno (if set)
    std::cerr << "Error: " << result.error().message << '\n';
    return 1;
}
auto cpu = std::move(result).value();
```

## Next Steps

- [Architecture](architecture.md) — Layered model, namespaces, and CMake targets
- [Roadmap](roadmap.md) — What's implemented and what's planned
- [Security Model](security.md) — Least-privilege policy and permission flags
- [Bindings Guide](bindings.md) — Python, LuaJIT, TypeScript, and C ABI usage
- [System & Hardware API](api/system_and_hardware.md)
- [File, Network, Software, Desktop & Security API](api/file_network_security.md)
- [Core, Application & Services API](api/core_app_services.md)

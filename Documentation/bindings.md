# Language Bindings & Public C ABI

All non-C++ language bindings target the **Public C ABI** (`libhapi_c.so` / `hapi_c.dll`), defined in [Bindings/c/include/hapi_c.h](../Bindings/c/include/hapi_c.h).

## C ABI Return Codes

| Constant | Value | Meaning |
| :--- | :--- | :--- |
| `HAPI_OK` | `0` | Success |
| `HAPI_ERR_INVALID_ARG` | `-1` | Null pointer or invalid argument |
| `HAPI_ERR_NOT_FOUND` | `-2` | File, process, or resource not found |
| `HAPI_ERR_PERMISSION` | `-3` | Denied by `SecurityPolicy` or OS |
| `HAPI_ERR_IO` | `-4` | I/O failure |
| `HAPI_ERR_UNSUPPORTED` | `-5` | Unsupported on current platform |
| `HAPI_ERR_UNKNOWN` | `-99` | Unclassified error |

## Python SDK

Located in `Bindings/python/hapi/__init__.py` (uses standard-library `ctypes`, no external packages required).

```python
from hapi import HapiClient

client = HapiClient()
os_info = client.get_os_info()
cpu = client.get_cpu_info()
mem = client.get_memory_info()
print(os_info.name, cpu.logical_cores, mem.total_kb)
```

Run the test script after building `libhapi_c.so`:

```bash
python3 Bindings/python/test_hapi.py
```

## LuaJIT SDK

Located in `Bindings/lua/hapi.lua` (uses LuaJIT `ffi`).

```lua
local hapi = require("hapi")
local os_info = hapi.get_os_info()
local cpu = hapi.get_cpu_info()
print(os_info.name, cpu.logical_cores)
```

Run the test script:

```bash
luajit Bindings/lua/test_hapi.lua
```

## TypeScript SDK

Located in `Bindings/typescript/` (`package.json`, `tsconfig.json`, `src/index.ts`), providing typed interfaces (`OsInfo`, `CpuInfo`, `MemoryInfo`, `DesktopInfo`, `UserInfo`, `HapiAdapter`) aligned with the C ABI and C++ SDK.

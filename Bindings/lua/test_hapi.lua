package.path = "./Bindings/lua/?.lua;" .. package.path
local hapi = require("hapi")

assert(hapi.version() == "0.1.0")
assert(hapi.platform() == "linux")

local os_info = hapi.get_os_info()
assert(os_info.kernel_name == "Linux")

local cpu = hapi.get_cpu_info()
assert(cpu.logical_cores > 0)

local mem = hapi.get_memory_info()
assert(mem.total_kb > 0)

print(string.format("Lua SDK OK: %s %s | %d threads | %.1f%% mem used",
    os_info.name, os_info.version, cpu.logical_cores, mem.usage_percent))

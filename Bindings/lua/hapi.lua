-- LuaJIT FFI SDK for API-on-Hyggshi-OS (HAPI)
local ffi = require("ffi")

ffi.cdef[[
typedef struct hapi_os_info_t {
    char name[128];
    char version[64];
    char id[64];
    char codename[64];
    char pretty_name[256];
    char kernel_name[64];
    char kernel_version[128];
    char architecture[64];
    char hostname[256];
} hapi_os_info_t;

typedef struct hapi_cpu_info_t {
    char model_name[256];
    char vendor_id[64];
    uint32_t physical_cores;
    uint32_t logical_cores;
    double frequency_mhz;
    uint64_t cache_size_kb;
} hapi_cpu_info_t;

typedef struct hapi_memory_info_t {
    uint64_t total_kb;
    uint64_t free_kb;
    uint64_t available_kb;
    uint64_t used_kb;
    uint64_t buffers_kb;
    uint64_t cached_kb;
    uint64_t swap_total_kb;
    uint64_t swap_free_kb;
    uint64_t swap_used_kb;
    double usage_percent;
} hapi_memory_info_t;

const char* hapi_version(void);
const char* hapi_platform_name(void);
int hapi_get_os_info(hapi_os_info_t* out);
int hapi_get_cpu_info(hapi_cpu_info_t* out);
int hapi_get_memory_info(hapi_memory_info_t* out);
]]

local function load_lib()
    local env_path = os.getenv("HAPI_C_LIB")
    if env_path and env_path ~= "" then
        return ffi.load(env_path)
    end
    local ok, lib = pcall(ffi.load, "./build/libhapi_c.so")
    if ok then return lib end
    return ffi.load("hapi_c")
end

local lib = load_lib()
local M = {}

function M.version()
    return ffi.string(lib.hapi_version())
end

function M.platform()
    return ffi.string(lib.hapi_platform_name())
end

function M.get_os_info()
    local info = ffi.new("hapi_os_info_t")
    local rc = lib.hapi_get_os_info(info)
    if rc ~= 0 then error("hapi_get_os_info failed: " .. tostring(rc)) end
    return {
        name = ffi.string(info.name),
        version = ffi.string(info.version),
        id = ffi.string(info.id),
        codename = ffi.string(info.codename),
        pretty_name = ffi.string(info.pretty_name),
        kernel_name = ffi.string(info.kernel_name),
        kernel_version = ffi.string(info.kernel_version),
        architecture = ffi.string(info.architecture),
        hostname = ffi.string(info.hostname),
    }
end

function M.get_cpu_info()
    local info = ffi.new("hapi_cpu_info_t")
    local rc = lib.hapi_get_cpu_info(info)
    if rc ~= 0 then error("hapi_get_cpu_info failed: " .. tostring(rc)) end
    return {
        model_name = ffi.string(info.model_name),
        vendor_id = ffi.string(info.vendor_id),
        physical_cores = tonumber(info.physical_cores),
        logical_cores = tonumber(info.logical_cores),
        frequency_mhz = tonumber(info.frequency_mhz),
        cache_size_kb = tonumber(info.cache_size_kb),
    }
end

function M.get_memory_info()
    local info = ffi.new("hapi_memory_info_t")
    local rc = lib.hapi_get_memory_info(info)
    if rc ~= 0 then error("hapi_get_memory_info failed: " .. tostring(rc)) end
    return {
        total_kb = tonumber(info.total_kb),
        available_kb = tonumber(info.available_kb),
        used_kb = tonumber(info.used_kb),
        usage_percent = tonumber(info.usage_percent),
    }
end

return M

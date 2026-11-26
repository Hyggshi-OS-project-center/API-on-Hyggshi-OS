#include <HAPI/system/memory_info.hpp>

#if defined(__linux__)
#include "linux/linux_adapters.hpp"
#endif

namespace hyggshios::system {

core::Result<MemoryInfo> try_get_memory_info() {
#if defined(__linux__)
    return detail::query_linux_memory_info();
#else
    return core::Result<MemoryInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Memory information adapter is not implemented for this platform",
        0,
    });
#endif
}

MemoryInfo get_memory_info() {
    auto res = try_get_memory_info();
    return res.ok() ? std::move(res).value() : MemoryInfo{};
}

} // namespace hyggshios::system

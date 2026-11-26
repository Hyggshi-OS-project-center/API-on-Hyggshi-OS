#include <HAPI/system/cpu_info.hpp>

#if defined(__linux__)
#include "linux/linux_adapters.hpp"
#endif

namespace hyggshios::system {

core::Result<CpuInfo> try_get_cpu_info() {
#if defined(__linux__)
    return detail::query_linux_cpu_info();
#else
    return core::Result<CpuInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "CPU information adapter is not implemented for this platform",
        0,
    });
#endif
}

CpuInfo get_cpu_info() {
    auto res = try_get_cpu_info();
    return res.ok() ? std::move(res).value() : CpuInfo{};
}

}  // namespace hyggshios::system

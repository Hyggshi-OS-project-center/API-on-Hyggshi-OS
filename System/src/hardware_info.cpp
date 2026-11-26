#include <HAPI/system/hardware_info.hpp>

#if defined(__linux__)
#include "linux/linux_adapters.hpp"
#endif

namespace hyggshios::system {

core::Result<HardwareInfo> try_get_hardware_info() {
#if defined(__linux__)
    return detail::query_linux_hardware_info();
#else
    return core::Result<HardwareInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Hardware information adapter is not implemented for this platform",
        0,
    });
#endif
}

HardwareInfo get_hardware_info() {
    auto res = try_get_hardware_info();
    return res.ok() ? std::move(res).value() : HardwareInfo{};
}

}  // namespace hyggshios::system

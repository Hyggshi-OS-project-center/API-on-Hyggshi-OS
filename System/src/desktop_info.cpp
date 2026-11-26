#include <HAPI/system/desktop_info.hpp>

#if defined(__linux__)
#include "linux/linux_adapters.hpp"
#endif

namespace hyggshios::system {

core::Result<DesktopInfo> try_get_desktop_info() {
#if defined(__linux__)
    return detail::query_linux_desktop_info();
#else
    return core::Result<DesktopInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Desktop environment adapter is not implemented for this platform",
        0,
    });
#endif
}

DesktopInfo get_desktop_info() {
    auto res = try_get_desktop_info();
    return res.ok() ? std::move(res).value() : DesktopInfo{};
}

core::Result<DesktopSettings> try_get_desktop_settings() {
#if defined(__linux__)
    return detail::query_linux_desktop_settings();
#else
    return core::Result<DesktopSettings>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Desktop settings adapter is not implemented for this platform",
        0,
    });
#endif
}

} // namespace hyggshios::system

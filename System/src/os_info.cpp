#include <HAPI/system/os_info.hpp>

#if defined(__linux__)
#include "linux/linux_os_info.hpp"
#endif

namespace hyggshios::system {

hyggshios::core::Result<OsInfo> try_get_os_info() {
#if defined(__linux__)
    return detail::query_linux_os_info();
#else
    return hyggshios::core::Result<OsInfo>::failure({
        hyggshios::core::ErrorCode::UnsupportedPlatform,
        "OS information adapter is not implemented for this platform",
        0,
    });
#endif
}

OsInfo get_os_info() {
    auto res = try_get_os_info();
    if (res.ok()) {
        return std::move(res).value();
    }
    return OsInfo{};
}

} // namespace hyggshios::system

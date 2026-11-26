#include <HAPI/system/process_info.hpp>

#if defined(__linux__)
#include "linux/linux_adapters.hpp"
#include <unistd.h>
#endif

namespace hyggshios::system {

core::Result<ProcessInfo> try_get_process_info(std::int32_t pid) {
#if defined(__linux__)
    return detail::query_linux_process_info(pid);
#else
    (void)pid;
    return core::Result<ProcessInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Process information adapter is not implemented for this platform",
        0,
    });
#endif
}

core::Result<ProcessInfo> try_get_current_process_info() {
#if defined(__linux__)
    return detail::query_linux_process_info(static_cast<std::int32_t>(::getpid()));
#else
    return core::Result<ProcessInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Process information adapter is not implemented for this platform",
        0,
    });
#endif
}

core::Result<std::vector<ProcessInfo>> try_list_processes() {
#if defined(__linux__)
    return detail::query_linux_process_list();
#else
    return core::Result<std::vector<ProcessInfo>>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Process listing adapter is not implemented for this platform",
        0,
    });
#endif
}

std::vector<ProcessInfo> list_processes() {
    auto res = try_list_processes();
    return res.ok() ? std::move(res).value() : std::vector<ProcessInfo>{};
}

} // namespace hyggshios::system

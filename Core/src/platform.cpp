#include <HAPI/core/platform.hpp>
#include <HAPI/core/types.hpp>

namespace hyggshios::core {

PlatformInfo get_platform_info() {
    PlatformInfo info;
    info.abi_version = std::string(kApiVersion);

#if defined(__EMSCRIPTEN__)
    info.type = PlatformType::Web;
    info.name = "web";
    info.is_posix = false;
    info.supports_native_syscalls = false;
#elif defined(_WIN32)
    info.type = PlatformType::Windows;
    info.name = "windows";
    info.is_posix = false;
    info.supports_native_syscalls = true;
#elif defined(__linux__)
    info.type = PlatformType::Linux;
    info.name = "linux";
    info.is_posix = true;
    info.supports_native_syscalls = true;
#else
    info.type = PlatformType::Unknown;
    info.name = "unknown";
    info.is_posix = false;
    info.supports_native_syscalls = false;
#endif

    return info;
}

} // namespace hyggshios::core

#include <HAPI/system/storage_info.hpp>

#if defined(__linux__)
#include "linux/linux_adapters.hpp"
#endif

namespace hyggshios::system {

core::Result<StorageInfo> try_get_storage_info(bool include_pseudo) {
#if defined(__linux__)
    return detail::query_linux_storage_info(include_pseudo);
#else
    (void)include_pseudo;
    return core::Result<StorageInfo>::failure({
        core::ErrorCode::UnsupportedPlatform,
        "Storage information adapter is not implemented for this platform",
        0,
    });
#endif
}

StorageInfo get_storage_info(bool include_pseudo) {
    auto res = try_get_storage_info(include_pseudo);
    return res.ok() ? std::move(res).value() : StorageInfo{};
}

}  // namespace hyggshios::system

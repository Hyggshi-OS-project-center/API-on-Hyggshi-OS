#pragma once

#include <HAPI/core/result.hpp>
#include <HAPI/system/os_info.hpp>

namespace hyggshios::system::detail {

/// Linux platform adapter for reading OS information from /etc/os-release, uname(), and gethostname().
[[nodiscard]] hyggshios::core::Result<OsInfo> query_linux_os_info();

}  // namespace hyggshios::system::detail

#pragma once

#include <HAPI/core/permissions.hpp>
#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace hyggshios::system::security {

/// Current user identity (never escalates privileges).
struct UserInfo {
    std::uint32_t uid{0};
    std::uint32_t euid{0};
    std::uint32_t gid{0};
    std::string username;
    std::string home_dir;
    std::string shell;
    bool is_root{false};
};

/// POSIX group entry.
struct GroupInfo {
    std::uint32_t gid{0};
    std::string name;
};

/// Linux process capability masks from /proc/self/status.
struct CapabilitiesInfo {
    std::string cap_inheritable;
    std::string cap_permitted;
    std::string cap_effective;
    std::string cap_bounding;
};

/// Security session metadata.
struct SessionSecurityInfo {
    std::string session_id;
    std::string tty;
    bool is_elevated{false};
};

[[nodiscard]] core::Result<UserInfo> try_get_current_user();
[[nodiscard]] core::Result<std::vector<GroupInfo>> try_get_current_groups();
[[nodiscard]] core::Result<CapabilitiesInfo> try_get_capabilities();
[[nodiscard]] core::Result<SessionSecurityInfo> try_get_session_info();
[[nodiscard]] core::SecurityPolicy get_security_policy();

} // namespace hyggshios::system::security

namespace hyggshi = hyggshios;

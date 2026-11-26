#include "linux_common.hpp"

#include <HAPI/core/permissions.hpp>
#include <HAPI/system/security_api.hpp>

#include <cstdlib>
#include <cstring>
#include <grp.h>
#include <pwd.h>
#include <sstream>
#include <unistd.h>

namespace hyggshios::system::security {

core::Result<UserInfo> try_get_current_user() {
    if (!core::has_permission(core::Permission::SecurityRead)) {
        return core::Result<UserInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SecurityRead is required",
            0,
        });
    }

    UserInfo info;
    info.uid = static_cast<std::uint32_t>(::getuid());
    info.euid = static_cast<std::uint32_t>(::geteuid());
    info.gid = static_cast<std::uint32_t>(::getgid());
    info.is_root = (info.euid == 0);

    struct passwd pwd{};
    struct passwd* result = nullptr;
    char buf[4096]{};
    if (::getpwuid_r(info.uid, &pwd, buf, sizeof(buf), &result) == 0 && result != nullptr) {
        info.username = pwd.pw_name != nullptr ? pwd.pw_name : "";
        info.home_dir = pwd.pw_dir != nullptr ? pwd.pw_dir : "";
        info.shell = pwd.pw_shell != nullptr ? pwd.pw_shell : "";
    }

    return core::Result<UserInfo>::success(std::move(info));
}

core::Result<std::vector<GroupInfo>> try_get_current_groups() {
    if (!core::has_permission(core::Permission::SecurityRead)) {
        return core::Result<std::vector<GroupInfo>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SecurityRead is required",
            0,
        });
    }

    const int count = ::getgroups(0, nullptr);
    if (count < 0) {
        return core::Result<std::vector<GroupInfo>>::failure({
            core::ErrorCode::SystemCallFailed,
            "getgroups() failed",
            errno,
        });
    }

    std::vector<gid_t> gids(static_cast<std::size_t>(count));
    if (count > 0 && ::getgroups(count, gids.data()) < 0) {
        return core::Result<std::vector<GroupInfo>>::failure({
            core::ErrorCode::SystemCallFailed,
            "getgroups() failed",
            errno,
        });
    }

    std::vector<GroupInfo> groups;
    groups.reserve(gids.size());
    for (const gid_t gid : gids) {
        GroupInfo g;
        g.gid = static_cast<std::uint32_t>(gid);
        struct group grp{};
        struct group* res = nullptr;
        char buf[4096]{};
        if (::getgrgid_r(gid, &grp, buf, sizeof(buf), &res) == 0 && res != nullptr &&
            grp.gr_name != nullptr) {
            g.name = grp.gr_name;
        }
        groups.push_back(std::move(g));
    }

    return core::Result<std::vector<GroupInfo>>::success(std::move(groups));
}

core::Result<CapabilitiesInfo> try_get_capabilities() {
    if (!core::has_permission(core::Permission::SecurityRead)) {
        return core::Result<CapabilitiesInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SecurityRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/self/status", content, err_no)) {
        return core::Result<CapabilitiesInfo>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/self/status",
            err_no,
        });
    }

    CapabilitiesInfo caps;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        const std::string_view key = detail::trim_ws(std::string_view(line).substr(0, colon));
        const std::string_view val = detail::trim_ws(std::string_view(line).substr(colon + 1));
        if (key == "CapInh") {
            caps.cap_inheritable = std::string(val);
        } else if (key == "CapPrm") {
            caps.cap_permitted = std::string(val);
        } else if (key == "CapEff") {
            caps.cap_effective = std::string(val);
        } else if (key == "CapBnd") {
            caps.cap_bounding = std::string(val);
        }
    }

    return core::Result<CapabilitiesInfo>::success(std::move(caps));
}

core::Result<SessionSecurityInfo> try_get_session_info() {
    if (!core::has_permission(core::Permission::SecurityRead)) {
        return core::Result<SessionSecurityInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SecurityRead is required",
            0,
        });
    }

    SessionSecurityInfo info;
    if (const char* sid = std::getenv("XDG_SESSION_ID")) {
        info.session_id = sid;
    }
    if (const char* tty = ::ttyname(STDIN_FILENO)) {
        info.tty = tty;
    }
    info.is_elevated = (::geteuid() == 0);
    return core::Result<SessionSecurityInfo>::success(std::move(info));
}

core::SecurityPolicy get_security_policy() {
    return core::active_security_policy();
}

} // namespace hyggshios::system::security

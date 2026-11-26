#include "linux_os_info.hpp"

#include <HAPI/core/permissions.hpp>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/utsname.h>
#include <unistd.h>
#include <unordered_map>

namespace hyggshios::system {

namespace {

std::string_view trim_whitespace(std::string_view sv) noexcept {
    while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t' || sv.front() == '\r')) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t' || sv.back() == '\r')) {
        sv.remove_suffix(1);
    }
    return sv;
}

std::string unquote_os_release_value(std::string_view raw) {
    raw = trim_whitespace(raw);
    if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
        std::string out;
        out.reserve(raw.size() - 2);
        for (std::size_t i = 1; i + 1 < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 2 < raw.size()) {
                const char next = raw[i + 1];
                if (next == '"' || next == '\\' || next == '$' || next == '`') {
                    out.push_back(next);
                    ++i;
                    continue;
                }
            }
            out.push_back(raw[i]);
        }
        return out;
    }

    if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'') {
        return std::string(raw.substr(1, raw.size() - 2));
    }

    return std::string(raw);
}

std::string lookup_field(const std::unordered_map<std::string, std::string>& fields,
                         std::string_view key) {
    if (const auto it = fields.find(std::string(key)); it != fields.end()) {
        return it->second;
    }
    return {};
}

bool read_file_to_string(const char* path, std::string& out, int& err_no) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        err_no = errno;
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (file.bad()) {
        err_no = errno;
        return false;
    }
    out = buffer.str();
    return true;
}

} // namespace

OsInfo parse_os_release(std::string_view content) {
    std::unordered_map<std::string, std::string> fields;

    std::size_t start = 0;
    while (start <= content.size()) {
        const std::size_t end = content.find('\n', start);
        const std::string_view line = trim_whitespace(
            end == std::string_view::npos ? content.substr(start) : content.substr(start, end - start));

        if (!line.empty() && line.front() != '#') {
            const std::size_t eq = line.find('=');
            if (eq != std::string_view::npos && eq > 0) {
                const std::string_view key = trim_whitespace(line.substr(0, eq));
                const std::string_view val = line.substr(eq + 1);
                if (!key.empty()) {
                    fields[std::string(key)] = unquote_os_release_value(val);
                }
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }

    OsInfo info;
    info.name = lookup_field(fields, "NAME");
    info.version = lookup_field(fields, "VERSION_ID");
    if (info.version.empty()) {
        info.version = lookup_field(fields, "VERSION");
    }
    info.id = lookup_field(fields, "ID");
    info.codename = lookup_field(fields, "VERSION_CODENAME");
    if (info.codename.empty()) {
        info.codename = lookup_field(fields, "UBUNTU_CODENAME");
    }
    if (info.codename.empty()) {
        info.codename = lookup_field(fields, "DEBIAN_CODENAME");
    }
    info.pretty_name = lookup_field(fields, "PRETTY_NAME");

    return info;
}

namespace detail {

hyggshios::core::Result<OsInfo> query_linux_os_info() {
    if (!hyggshios::core::has_permission(hyggshios::core::Permission::SystemInfoRead)) {
        return hyggshios::core::Result<OsInfo>::failure({
            hyggshios::core::ErrorCode::PermissionDenied,
            "Permission denied: SystemInfoRead is required",
            0,
        });
    }

    std::string os_release_content;
    int err_no = 0;
    if (!read_file_to_string("/etc/os-release", os_release_content, err_no) &&
        !read_file_to_string("/usr/lib/os-release", os_release_content, err_no)) {
        return hyggshios::core::Result<OsInfo>::failure({
            hyggshios::core::ErrorCode::NotFound,
            "Failed to read /etc/os-release or /usr/lib/os-release: " +
                std::string(std::strerror(err_no)),
            err_no,
        });
    }

    OsInfo info = parse_os_release(os_release_content);

    struct utsname uts{};
    if (::uname(&uts) != 0) {
        const int uname_err = errno;
        return hyggshios::core::Result<OsInfo>::failure({
            hyggshios::core::ErrorCode::SystemCallFailed,
            "uname() failed: " + std::string(std::strerror(uname_err)),
            uname_err,
        });
    }
    info.kernel_name = uts.sysname;
    info.kernel_version = uts.release;
    info.architecture = uts.machine;

    char host_buf[256]{};
    if (::gethostname(host_buf, sizeof(host_buf)) != 0) {
        const int host_err = errno;
        return hyggshios::core::Result<OsInfo>::failure({
            hyggshios::core::ErrorCode::SystemCallFailed,
            "gethostname() failed: " + std::string(std::strerror(host_err)),
            host_err,
        });
    }
    host_buf[sizeof(host_buf) - 1] = '\0';
    info.hostname = host_buf;

    return hyggshios::core::Result<OsInfo>::success(std::move(info));
}

} // namespace detail
} // namespace hyggshios::system

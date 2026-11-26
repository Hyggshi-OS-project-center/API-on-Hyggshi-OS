#include "hapi_c.h"

#include <HAPI/application/notification.hpp>
#include <HAPI/core/platform.hpp>
#include <HAPI/core/types.hpp>
#include <HAPI/system/cpu_info.hpp>
#include <HAPI/system/desktop_info.hpp>
#include <HAPI/system/file_api.hpp>
#include <HAPI/system/hardware_info.hpp>
#include <HAPI/system/memory_info.hpp>
#include <HAPI/system/os_info.hpp>
#include <HAPI/system/runtime_info.hpp>
#include <HAPI/system/security_api.hpp>

#include <algorithm>
#include <cstring>
#include <string>

namespace {

template <std::size_t N>
void copy_str(char (&dst)[N], const std::string& src) noexcept {
    std::memset(dst, 0, N);
    const std::size_t len = std::min(src.size(), N - 1);
    std::memcpy(dst, src.data(), len);
}

int map_error(hyggshios::core::ErrorCode code) noexcept {
    switch (code) {
        case hyggshios::core::ErrorCode::None:
            return HAPI_OK;
        case hyggshios::core::ErrorCode::InvalidArgument:
            return HAPI_ERR_INVALID_ARG;
        case hyggshios::core::ErrorCode::NotFound:
            return HAPI_ERR_NOT_FOUND;
        case hyggshios::core::ErrorCode::PermissionDenied:
            return HAPI_ERR_PERMISSION;
        case hyggshios::core::ErrorCode::IoError:
            return HAPI_ERR_IO;
        case hyggshios::core::ErrorCode::UnsupportedPlatform:
            return HAPI_ERR_UNSUPPORTED;
        default:
            return HAPI_ERR_UNKNOWN;
    }
}

}  // namespace

extern "C" {

const char* hapi_version(void) {
    return "0.1.0";
}

const char* hapi_platform_name(void) {
    static const std::string name = hyggshios::core::get_platform_info().name;
    return name.c_str();
}

int hapi_get_os_info(hapi_os_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::try_get_os_info();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& v = res.value();
    copy_str(out->name, v.name);
    copy_str(out->version, v.version);
    copy_str(out->id, v.id);
    copy_str(out->codename, v.codename);
    copy_str(out->pretty_name, v.pretty_name);
    copy_str(out->kernel_name, v.kernel_name);
    copy_str(out->kernel_version, v.kernel_version);
    copy_str(out->architecture, v.architecture);
    copy_str(out->hostname, v.hostname);
    return HAPI_OK;
}

int hapi_get_cpu_info(hapi_cpu_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::try_get_cpu_info();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& v = res.value();
    copy_str(out->model_name, v.model_name);
    copy_str(out->vendor_id, v.vendor_id);
    out->physical_cores = v.physical_cores;
    out->logical_cores = v.logical_cores;
    out->frequency_mhz = v.frequency_mhz;
    out->cache_size_kb = v.cache_size_kb;
    return HAPI_OK;
}

int hapi_get_memory_info(hapi_memory_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::try_get_memory_info();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& v = res.value();
    out->total_kb = v.total_kb;
    out->free_kb = v.free_kb;
    out->available_kb = v.available_kb;
    out->used_kb = v.used_kb;
    out->buffers_kb = v.buffers_kb;
    out->cached_kb = v.cached_kb;
    out->swap_total_kb = v.swap_total_kb;
    out->swap_free_kb = v.swap_free_kb;
    out->swap_used_kb = v.swap_used_kb;
    out->usage_percent = v.usage_percent;
    return HAPI_OK;
}

int hapi_get_uptime_info(hapi_uptime_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::try_get_uptime_info();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    out->uptime_seconds = res.value().uptime_seconds;
    out->idle_seconds = res.value().idle_seconds;
    return HAPI_OK;
}

int hapi_get_desktop_info(hapi_desktop_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::try_get_desktop_info();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& v = res.value();
    copy_str(out->desktop_environment, v.desktop_environment);
    copy_str(out->session_type, v.session_type);
    copy_str(out->display, v.display);
    copy_str(out->locale, v.locale);
    out->is_graphical = v.is_graphical ? 1 : 0;
    return HAPI_OK;
}

int hapi_get_hardware_info(hapi_hardware_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::try_get_hardware_info();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& v = res.value();
    copy_str(out->vendor, v.vendor);
    copy_str(out->product_name, v.product_name);
    copy_str(out->product_version, v.product_version);
    copy_str(out->board_vendor, v.board_vendor);
    copy_str(out->board_name, v.board_name);
    copy_str(out->bios_vendor, v.bios_vendor);
    copy_str(out->bios_version, v.bios_version);
    copy_str(out->chassis_type, v.chassis_type);
    return HAPI_OK;
}

int hapi_get_user_info(hapi_user_info_t* out) {
    if (out == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::security::try_get_current_user();
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& v = res.value();
    out->uid = v.uid;
    out->euid = v.euid;
    out->gid = v.gid;
    copy_str(out->username, v.username);
    copy_str(out->home_dir, v.home_dir);
    copy_str(out->shell, v.shell);
    out->is_root = v.is_root ? 1 : 0;
    return HAPI_OK;
}

int hapi_file_exists(const char* path) {
    if (path == nullptr) {
        return 0;
    }
    return hyggshios::system::file::exists(path) ? 1 : 0;
}

int hapi_file_read_text(const char* path, char* buf, size_t buf_size, size_t* out_len) {
    if (path == nullptr || buf == nullptr || buf_size == 0) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::file::read_text(path);
    if (!res.ok()) {
        return map_error(res.error().code);
    }
    const auto& text = res.value();
    const std::size_t copy_len = std::min(text.size(), buf_size - 1);
    std::memcpy(buf, text.data(), copy_len);
    buf[copy_len] = '\0';
    if (out_len != nullptr) {
        *out_len = text.size();
    }
    return HAPI_OK;
}

int hapi_file_write_text(const char* path, const char* content) {
    if (path == nullptr || content == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    auto res = hyggshios::system::file::write_text(path, content);
    return res.ok() ? HAPI_OK : map_error(res.error().code);
}

int hapi_send_notification(const char* app_name, const char* title, const char* body) {
    if (title == nullptr) {
        return HAPI_ERR_INVALID_ARG;
    }
    hyggshios::application::NotificationRequest req;
    if (app_name != nullptr) {
        req.app_name = app_name;
    }
    req.title = title;
    if (body != nullptr) {
        req.body = body;
    }
    auto res = hyggshios::application::send_notification(req);
    return res.ok() ? HAPI_OK : map_error(res.error().code);
}

}  // extern "C"

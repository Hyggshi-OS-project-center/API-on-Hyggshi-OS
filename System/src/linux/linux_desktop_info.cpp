#include <HAPI/core/permissions.hpp>

#include <cstdlib>
#include <string>

#include "linux_adapters.hpp"

namespace hyggshios::system::detail {

namespace {

std::string get_env_str(const char* name) {
    const char* val = std::getenv(name);
    return val != nullptr ? std::string(val) : std::string{};
}

}  // namespace

core::Result<DesktopInfo> query_linux_desktop_info() {
    if (!core::has_permission(core::Permission::SystemInfoRead)) {
        return core::Result<DesktopInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SystemInfoRead is required",
            0,
        });
    }

    DesktopInfo info;

    info.desktop_environment = get_env_str("XDG_CURRENT_DESKTOP");
    if (info.desktop_environment.empty()) {
        info.desktop_environment = get_env_str("XDG_SESSION_DESKTOP");
    }
    if (info.desktop_environment.empty()) {
        info.desktop_environment = get_env_str("DESKTOP_SESSION");
    }

    const std::string wayland_display = get_env_str("WAYLAND_DISPLAY");
    const std::string x11_display = get_env_str("DISPLAY");

    info.session_type = get_env_str("XDG_SESSION_TYPE");
    if (info.session_type.empty()) {
        if (!wayland_display.empty()) {
            info.session_type = "wayland";
        } else if (!x11_display.empty()) {
            info.session_type = "x11";
        } else {
            info.session_type = "tty";
        }
    }

    if (!wayland_display.empty()) {
        info.display = wayland_display;
    } else {
        info.display = x11_display;
    }

    info.locale = get_env_str("LC_ALL");
    if (info.locale.empty()) {
        info.locale = get_env_str("LANG");
    }

    info.is_graphical = !wayland_display.empty() || !x11_display.empty() ||
                        info.session_type == "wayland" || info.session_type == "x11";

    return core::Result<DesktopInfo>::success(std::move(info));
}

core::Result<DesktopSettings> query_linux_desktop_settings() {
    if (!core::has_permission(core::Permission::SystemInfoRead)) {
        return core::Result<DesktopSettings>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SystemInfoRead is required",
            0,
        });
    }

    DesktopSettings settings;
    settings.gtk_theme = get_env_str("GTK_THEME");

    auto run_gsettings = [](const char* cmd) -> std::string {
        FILE* pipe = ::popen(cmd, "r");
        if (pipe == nullptr) {
            return {};
        }
        char buf[512]{};
        std::string out;
        if (std::fgets(buf, sizeof(buf), pipe) != nullptr) {
            out = buf;
        }
        ::pclose(pipe);
        while (!out.empty() && (out.back() == '\n' || out.back() == '\r' || out.back() == '\'')) {
            out.pop_back();
        }
        if (!out.empty() && out.front() == '\'') {
            out.erase(out.begin());
        }
        return out;
    };

    if (settings.gtk_theme.empty()) {
        settings.gtk_theme =
            run_gsettings("gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null");
    }
    settings.icon_theme =
        run_gsettings("gsettings get org.gnome.desktop.interface icon-theme 2>/dev/null");
    settings.color_scheme =
        run_gsettings("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null");
    settings.wallpaper_uri =
        run_gsettings("gsettings get org.gnome.desktop.background picture-uri 2>/dev/null");
    settings.panel_position = run_gsettings(
        "gsettings get org.gnome.shell.extensions.dash-to-dock dock-position 2>/dev/null");

    return core::Result<DesktopSettings>::success(std::move(settings));
}

}  // namespace hyggshios::system::detail

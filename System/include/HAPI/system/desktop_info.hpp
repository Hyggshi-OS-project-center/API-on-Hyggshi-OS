#pragma once

#include <HAPI/core/result.hpp>

#include <string>

namespace hyggshios::system {

/// Desktop environment and graphical session metadata.
struct DesktopInfo {
    std::string desktop_environment; ///< Desktop name (e.g., "GNOME", "KDE", "Hyprland")
    std::string session_type;        ///< Session protocol (e.g., "wayland", "x11", "tty")
    std::string display;             ///< Display socket/identifier (e.g., "wayland-0", ":0")
    std::string locale;              ///< Active system locale (e.g., "en_US.UTF-8")
    bool is_graphical{false};        ///< True if a graphical display session is active
};

/// Desktop theme, wallpaper, and UI appearance settings.
struct DesktopSettings {
    std::string gtk_theme;
    std::string icon_theme;
    std::string color_scheme;  ///< e.g., "prefer-dark", "default"
    std::string wallpaper_uri;
    std::string panel_position;
};

/// Retrieves desktop environment and session information for the current user session.
[[nodiscard]] core::Result<DesktopInfo> try_get_desktop_info();

/// Retrieves desktop environment and session information (best-effort on failure).
[[nodiscard]] DesktopInfo get_desktop_info();

/// Retrieves desktop theme, wallpaper, and appearance settings (best-effort).
[[nodiscard]] core::Result<DesktopSettings> try_get_desktop_settings();

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

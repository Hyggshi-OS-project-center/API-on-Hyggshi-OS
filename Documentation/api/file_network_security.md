# File, Network, Software, Desktop & Security API Reference

## File API (`<HAPI/system/file_api.hpp>`)

Namespace: `hyggshios::system::file`

- `read_text(path)`, `read_bytes(path)`
- `write_text(path, content)`, `write_bytes(path, data)` (emits `"file.written"` on `EventBus::global()`)
- `copy(src_path, dst_path, overwrite)`
- `remove(path)` (emits `"file.deleted"`)
- `exists(path)`, `stat(path)`
- `watch_once(path, last_mtime_ns, callback)` (emits `"file.changed"`)

## Network API (`<HAPI/system/network_api.hpp>`)

Namespace: `hyggshios::system::network`

- `try_get_interfaces()` — IPv4/IPv6 addresses, MAC, up/loopback flags (`getifaddrs`)
- `try_get_dns_config()` — Nameservers and search domains (`/etc/resolv.conf`)
- `try_get_connections()` — Active TCP sockets and states (`/proc/net/tcp`)
- `try_get_wifi_info()` — Wireless link and signal level (`/proc/net/wireless`)
- `get_proxy_config()` — `http_proxy`, `https_proxy`, `no_proxy`
- `try_get_statistics()` — Per-interface RX/TX byte and packet counters (`/proc/net/dev`)

## Software API (`<HAPI/system/software_api.hpp>`)

Namespace: `hyggshios::system::software`

- `try_list_installed_packages(max_count)` / `parse_dpkg_status()` (`/var/lib/dpkg/status`)
- `get_package_manager_info()` — Detects `apt`, `dpkg`, `pacman`, `dnf`, `flatpak`, `snap`
- `try_list_loaded_libraries()` — Shared objects loaded in the current process (`/proc/self/maps`)

## Desktop API (`<HAPI/system/desktop_info.hpp>`)

Namespace: `hyggshios::system`

- `try_get_desktop_info()` / `get_desktop_info()` — Desktop environment, session type (`wayland`/`x11`/`tty`), display, locale
- `try_get_desktop_settings()` — GTK theme, icon theme, color scheme, wallpaper URI, panel position

## Security API (`<HAPI/system/security_api.hpp>`)

Namespace: `hyggshios::system::security`

- `try_get_current_user()` — UID, EUID, GID, username, home, shell, `is_root`
- `try_get_current_groups()` — POSIX group IDs and names
- `try_get_capabilities()` — `CapInh`, `CapPrm`, `CapEff`, `CapBnd` from `/proc/self/status`
- `try_get_session_info()` — `XDG_SESSION_ID`, TTY, elevation status
- `get_security_policy()` — Active `core::SecurityPolicy`

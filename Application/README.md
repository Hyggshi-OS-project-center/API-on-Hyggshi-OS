# Application Layer

Application-level APIs: current process metadata, installed `.desktop` app discovery, and desktop notifications.

## Status

Implemented.

## APIs

- `try_get_current_app()` — PID, executable path (`/proc/self/exe`), working directory
- `try_list_installed_apps(max_count)` — Scans `/usr/share/applications` and `~/.local/share/applications`
- `send_notification(NotificationRequest)` — Emits on `EventBus::global("desktop.notification")` and invokes `notify-send`

See [Documentation/api/core_app_services.md](../Documentation/api/core_app_services.md).

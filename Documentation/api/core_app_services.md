# Core, Application & Services API Reference

## Core (`<HAPI/core/...>`)

- **`error.hpp` & `result.hpp`**: `ErrorCode`, `Error`, and `Result<T>` (`ok()`, `value()`, `error()`, `value_or()`)
- **`permissions.hpp`**: `Permission` flags, `SecurityPolicy` (`allow_privilege_escalation = false` by default), `active_security_policy()`, `has_permission(perm)`
- **`event.hpp`**: `Event`, `EventBus::global()` (`subscribe(topic, handler)`, `unsubscribe(id)`, `emit(event)`)
- **`platform.hpp`**: `PlatformType` (`Linux`, `Windows`, `Web`, `Unknown`), `get_platform_info()`

## Application (`<HAPI/application/...>`)

Namespace: `hyggshios::application`

- **`app_info.hpp`**:
  - `try_get_current_app()` — Current PID, executable path (`/proc/self/exe`), working directory
  - `try_list_installed_apps(max_count)` — Scans `/usr/share/applications` and `~/.local/share/applications` `.desktop` entries
- **`notification.hpp`**:
  - `send_notification(NotificationRequest)` — Emits `"desktop.notification"` on `EventBus::global()` and invokes `notify-send` when running in a graphical session

## Services (`<HAPI/services/service_manager.hpp>`)

Namespace: `hyggshios::services`

- `try_list_services(max_count)` — Read-only listing of systemd service units
- `try_get_service_status(unit_name)` — Queries load/active/sub state for a specific unit

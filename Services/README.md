# Services Layer

Service-facing APIs: read-only access to systemd service units.

## Status

Implemented.

## APIs

- `try_list_services(max_count)` — Lists systemd units via `systemctl list-units`
- `try_get_service_status(unit_name)` — Queries load/active/sub state for a specific unit

See [Documentation/api/core_app_services.md](../Documentation/api/core_app_services.md).

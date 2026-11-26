#pragma once

#include <HAPI/core/result.hpp>

#include <string>
#include <vector>

namespace hyggshios::services {

/// Status entry for a systemd unit/service.
struct ServiceInfo {
    std::string name;
    std::string load_state;
    std::string active_state;
    std::string sub_state;
    std::string description;
};

/// Lists systemd services (read-only).
[[nodiscard]] core::Result<std::vector<ServiceInfo>> try_list_services(std::size_t max_count = 100);

/// Queries status for a specific systemd service by unit name.
[[nodiscard]] core::Result<ServiceInfo> try_get_service_status(const std::string& unit_name);

} // namespace hyggshios::services

namespace hyggshi = hyggshios;

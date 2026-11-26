#include <HAPI/core/permissions.hpp>
#include <HAPI/services/service_manager.hpp>

#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>

namespace hyggshios::services {

core::Result<std::vector<ServiceInfo>> try_list_services(std::size_t max_count) {
    if (!core::has_permission(core::Permission::SoftwareRead)) {
        return core::Result<std::vector<ServiceInfo>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SoftwareRead is required",
            0,
        });
    }

    std::vector<ServiceInfo> services;
    if (!std::filesystem::exists("/usr/bin/systemctl") &&
        !std::filesystem::exists("/bin/systemctl")) {
        return core::Result<std::vector<ServiceInfo>>::success(std::move(services));
    }

    FILE* pipe = ::popen(
        "systemctl list-units --type=service --all --no-pager --no-legend 2>/dev/null", "r");
    if (pipe == nullptr) {
        return core::Result<std::vector<ServiceInfo>>::success(std::move(services));
    }

    char buf[1024]{};
    while (std::fgets(buf, sizeof(buf), pipe) != nullptr) {
        std::istringstream iss(buf);
        ServiceInfo info;
        if (!(iss >> info.name >> info.load_state >> info.active_state >> info.sub_state)) {
            continue;
        }
        std::getline(iss >> std::ws, info.description);
        services.push_back(std::move(info));
        if (max_count > 0 && services.size() >= max_count) {
            break;
        }
    }
    ::pclose(pipe);

    return core::Result<std::vector<ServiceInfo>>::success(std::move(services));
}

core::Result<ServiceInfo> try_get_service_status(const std::string& unit_name) {
    if (!core::has_permission(core::Permission::SoftwareRead)) {
        return core::Result<ServiceInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: SoftwareRead is required",
            0,
        });
    }
    if (unit_name.empty()) {
        return core::Result<ServiceInfo>::failure({
            core::ErrorCode::InvalidArgument,
            "Unit name cannot be empty",
            0,
        });
    }

    auto list_res = try_list_services(0);
    if (!list_res.ok()) {
        return core::Result<ServiceInfo>::failure(list_res.error());
    }

    for (const auto& svc : list_res.value()) {
        if (svc.name == unit_name || svc.name == (unit_name + ".service")) {
            return core::Result<ServiceInfo>::success(svc);
        }
    }

    return core::Result<ServiceInfo>::failure({
        core::ErrorCode::NotFound,
        "Service not found: " + unit_name,
        0,
    });
}

} // namespace hyggshios::services

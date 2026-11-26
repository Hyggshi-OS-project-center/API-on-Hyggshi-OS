#include "linux_adapters.hpp"
#include "linux_common.hpp"

#include <HAPI/core/permissions.hpp>

#include <string>

namespace hyggshios::system {

std::string decode_chassis_type(std::string_view raw_code) {
    int code = 0;
    if (!detail::parse_int(raw_code, code)) {
        return std::string(detail::trim_ws(raw_code));
    }

    switch (code) {
        case 1:
            return "Other";
        case 3:
            return "Desktop";
        case 4:
            return "Low Profile Desktop";
        case 5:
            return "Pizza Box";
        case 6:
            return "Mini Tower";
        case 7:
            return "Tower";
        case 8:
            return "Portable";
        case 9:
            return "Laptop";
        case 10:
            return "Notebook";
        case 11:
            return "Hand Held";
        case 12:
            return "Docking Station";
        case 13:
            return "All in One";
        case 14:
            return "Sub Notebook";
        case 15:
            return "Space-saving";
        case 17:
            return "Main Server Chassis";
        case 23:
            return "Rack Mount Chassis";
        case 30:
            return "Tablet";
        case 31:
            return "Convertible";
        case 32:
            return "Detachable";
        default:
            return "Unknown (" + std::to_string(code) + ")";
    }
}

namespace detail {

namespace {

std::string read_dmi_attr(const char* filename) {
    const std::string path = std::string("/sys/class/dmi/id/") + filename;
    std::string content;
    int err_no = 0;
    if (read_file_text(path.c_str(), content, err_no)) {
        return std::string(trim_ws(content));
    }
    return {};
}

} // namespace

core::Result<HardwareInfo> query_linux_hardware_info() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<HardwareInfo>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    HardwareInfo info;
    info.vendor = read_dmi_attr("sys_vendor");
    info.product_name = read_dmi_attr("product_name");
    info.product_version = read_dmi_attr("product_version");
    info.board_vendor = read_dmi_attr("board_vendor");
    info.board_name = read_dmi_attr("board_name");
    info.bios_vendor = read_dmi_attr("bios_vendor");
    info.bios_version = read_dmi_attr("bios_version");

    const std::string raw_chassis = read_dmi_attr("chassis_type");
    if (!raw_chassis.empty()) {
        info.chassis_type = decode_chassis_type(raw_chassis);
    }

    return core::Result<HardwareInfo>::success(std::move(info));
}

} // namespace detail
} // namespace hyggshios::system

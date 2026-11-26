#include "linux_common.hpp"

#include <HAPI/core/permissions.hpp>
#include <HAPI/system/hardware_devices.hpp>

#include <cctype>
#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace hyggshios::system {

namespace {

std::string read_sys_attr(const fs::path& path) {
    std::string text;
    int err_no = 0;
    if (detail::read_file_text(path.c_str(), text, err_no)) {
        return std::string(detail::trim_ws(text));
    }
    return {};
}

std::string read_driver_basename(const fs::path& driver_link) {
    std::error_code ec;
    if (fs::is_symlink(driver_link, ec)) {
        const fs::path target = fs::read_symlink(driver_link, ec);
        if (!ec) {
            return target.filename().string();
        }
    }
    return {};
}

bool is_card_dir(const std::string& name) {
    if (name.rfind("card", 0) != 0 || name.size() <= 4) {
        return false;
    }
    for (std::size_t i = 4; i < name.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
            return false;
        }
    }
    return true;
}

} // namespace

core::Result<std::vector<GpuDevice>> try_get_gpu_devices() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<GpuDevice>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<GpuDevice> gpus;
    std::error_code ec;
    const fs::path drm_dir("/sys/class/drm");
    if (!fs::exists(drm_dir, ec)) {
        return core::Result<std::vector<GpuDevice>>::success(std::move(gpus));
    }

    for (const auto& entry : fs::directory_iterator(drm_dir, ec)) {
        const std::string name = entry.path().filename().string();
        if (!is_card_dir(name)) {
            continue;
        }
        const fs::path dev_path = entry.path() / "device";
        if (!fs::exists(dev_path, ec)) {
            continue;
        }
        GpuDevice gpu;
        gpu.card = name;
        gpu.vendor_id = read_sys_attr(dev_path / "vendor");
        gpu.device_id = read_sys_attr(dev_path / "device");
        gpu.driver = read_driver_basename(dev_path / "driver");
        gpus.push_back(std::move(gpu));
    }

    return core::Result<std::vector<GpuDevice>>::success(std::move(gpus));
}

core::Result<std::vector<BatteryInfo>> try_get_batteries() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<BatteryInfo>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<BatteryInfo> batteries;
    std::error_code ec;
    const fs::path ps_dir("/sys/class/power_supply");
    if (!fs::exists(ps_dir, ec)) {
        return core::Result<std::vector<BatteryInfo>>::success(std::move(batteries));
    }

    for (const auto& entry : fs::directory_iterator(ps_dir, ec)) {
        const std::string type = read_sys_attr(entry.path() / "type");
        if (type != "Battery") {
            continue;
        }
        BatteryInfo bat;
        bat.name = entry.path().filename().string();
        bat.status = read_sys_attr(entry.path() / "status");
        const std::string cap = read_sys_attr(entry.path() / "capacity");
        detail::parse_int(cap, bat.capacity_pct);
        bat.present = read_sys_attr(entry.path() / "present") == "1";
        batteries.push_back(std::move(bat));
    }

    return core::Result<std::vector<BatteryInfo>>::success(std::move(batteries));
}

core::Result<std::vector<AudioDevice>> try_get_audio_devices() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<AudioDevice>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<AudioDevice> cards;
    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/asound/cards", content, err_no)) {
        return core::Result<std::vector<AudioDevice>>::success(std::move(cards));
    }

    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        const std::string_view sv = detail::trim_ws(line);
        if (sv.empty() || !std::isdigit(static_cast<unsigned char>(sv.front()))) {
            continue;
        }
        const std::size_t bracket_open = sv.find('[');
        const std::size_t bracket_close = sv.find(']');
        const std::size_t dash = sv.find(" - ");
        if (bracket_open != std::string_view::npos && bracket_close != std::string_view::npos) {
            AudioDevice dev;
            detail::parse_leading_int(sv, dev.index);
            dev.id = std::string(
                detail::trim_ws(sv.substr(bracket_open + 1, bracket_close - bracket_open - 1)));
            if (dash != std::string_view::npos) {
                dev.name = std::string(detail::trim_ws(sv.substr(dash + 3)));
            }
            cards.push_back(std::move(dev));
        }
    }

    return core::Result<std::vector<AudioDevice>>::success(std::move(cards));
}

core::Result<std::vector<DisplayOutput>> try_get_display_outputs() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<DisplayOutput>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<DisplayOutput> outputs;
    std::error_code ec;
    const fs::path drm_dir("/sys/class/drm");
    if (!fs::exists(drm_dir, ec)) {
        return core::Result<std::vector<DisplayOutput>>::success(std::move(outputs));
    }

    for (const auto& entry : fs::directory_iterator(drm_dir, ec)) {
        const fs::path status_path = entry.path() / "status";
        if (!fs::exists(status_path, ec)) {
            continue;
        }
        DisplayOutput disp;
        disp.connector = entry.path().filename().string();
        disp.status = read_sys_attr(status_path);
        disp.enabled = read_sys_attr(entry.path() / "enabled") == "enabled";
        outputs.push_back(std::move(disp));
    }

    return core::Result<std::vector<DisplayOutput>>::success(std::move(outputs));
}

core::Result<std::vector<UsbDevice>> try_get_usb_devices() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<UsbDevice>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<UsbDevice> devices;
    std::error_code ec;
    const fs::path usb_dir("/sys/bus/usb/devices");
    if (!fs::exists(usb_dir, ec)) {
        return core::Result<std::vector<UsbDevice>>::success(std::move(devices));
    }

    for (const auto& entry : fs::directory_iterator(usb_dir, ec)) {
        const std::string id_vendor = read_sys_attr(entry.path() / "idVendor");
        if (id_vendor.empty()) {
            continue;
        }
        UsbDevice dev;
        dev.bus_id = entry.path().filename().string();
        dev.vendor_id = id_vendor;
        dev.product_id = read_sys_attr(entry.path() / "idProduct");
        dev.manufacturer = read_sys_attr(entry.path() / "manufacturer");
        dev.product = read_sys_attr(entry.path() / "product");
        devices.push_back(std::move(dev));
    }

    return core::Result<std::vector<UsbDevice>>::success(std::move(devices));
}

core::Result<std::vector<PciDevice>> try_get_pci_devices() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<PciDevice>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<PciDevice> devices;
    std::error_code ec;
    const fs::path pci_dir("/sys/bus/pci/devices");
    if (!fs::exists(pci_dir, ec)) {
        return core::Result<std::vector<PciDevice>>::success(std::move(devices));
    }

    for (const auto& entry : fs::directory_iterator(pci_dir, ec)) {
        PciDevice dev;
        dev.slot = entry.path().filename().string();
        dev.vendor_id = read_sys_attr(entry.path() / "vendor");
        dev.device_id = read_sys_attr(entry.path() / "device");
        dev.class_id = read_sys_attr(entry.path() / "class");
        dev.driver = read_driver_basename(entry.path() / "driver");
        devices.push_back(std::move(dev));
    }

    return core::Result<std::vector<PciDevice>>::success(std::move(devices));
}

core::Result<std::vector<SensorReading>> try_get_sensor_readings() {
    if (!core::has_permission(core::Permission::HardwareInfoRead)) {
        return core::Result<std::vector<SensorReading>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: HardwareInfoRead is required",
            0,
        });
    }

    std::vector<SensorReading> readings;
    std::error_code ec;
    const fs::path hwmon_dir("/sys/class/hwmon");
    if (!fs::exists(hwmon_dir, ec)) {
        return core::Result<std::vector<SensorReading>>::success(std::move(readings));
    }

    for (const auto& chip_dir : fs::directory_iterator(hwmon_dir, ec)) {
        const std::string chip_name = read_sys_attr(chip_dir.path() / "name");
        for (const auto& attr : fs::directory_iterator(chip_dir.path(), ec)) {
            const std::string fname = attr.path().filename().string();
            if (fname.rfind("temp", 0) == 0 && fname.ends_with("_input")) {
                std::int64_t milli_c = 0;
                if (detail::parse_int(read_sys_attr(attr.path()), milli_c)) {
                    const std::string prefix = fname.substr(0, fname.size() - 6);
                    std::string label = read_sys_attr(chip_dir.path() / (prefix + "_label"));
                    if (label.empty()) {
                        label = prefix;
                    }
                    readings.push_back(SensorReading{
                        chip_name,
                        std::move(label),
                        static_cast<double>(milli_c) / 1000.0,
                        "C",
                    });
                }
            }
        }
    }

    return core::Result<std::vector<SensorReading>>::success(std::move(readings));
}

} // namespace hyggshios::system

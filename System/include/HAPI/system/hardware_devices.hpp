#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace hyggshios::system {

/// Graphics adapter metadata from /sys/class/drm.
struct GpuDevice {
    std::string card;      ///< e.g., "card0"
    std::string vendor_id; ///< PCI vendor hex (e.g., "0x8086")
    std::string device_id; ///< PCI device hex (e.g., "0x9a49")
    std::string driver;    ///< Kernel driver name (e.g., "i915", "amdgpu", "nvidia")
};

/// Battery power supply status from /sys/class/power_supply.
struct BatteryInfo {
    std::string name;           ///< e.g., "BAT0"
    std::string status;         ///< "Charging", "Discharging", "Full", "Not charging"
    std::uint32_t capacity_pct{0};
    bool present{false};
};

/// Audio card entry from /proc/asound/cards.
struct AudioDevice {
    std::int32_t index{0};
    std::string id;
    std::string name;
};

/// Display connector metadata from /sys/class/drm.
struct DisplayOutput {
    std::string connector; ///< e.g., "card0-eDP-1", "card0-HDMI-A-1"
    std::string status;    ///< "connected" or "disconnected"
    bool enabled{false};
};

/// USB device entry from /sys/bus/usb/devices.
struct UsbDevice {
    std::string bus_id;
    std::string vendor_id;
    std::string product_id;
    std::string manufacturer;
    std::string product;
};

/// PCI device entry from /sys/bus/pci/devices.
struct PciDevice {
    std::string slot;
    std::string vendor_id;
    std::string device_id;
    std::string class_id;
    std::string driver;
};

/// Hardware thermal/voltage/fan sensor reading from /sys/class/hwmon.
struct SensorReading {
    std::string chip;
    std::string label;
    double value{0.0};
    std::string unit; ///< e.g., "C", "RPM", "V"
};

[[nodiscard]] core::Result<std::vector<GpuDevice>> try_get_gpu_devices();
[[nodiscard]] core::Result<std::vector<BatteryInfo>> try_get_batteries();
[[nodiscard]] core::Result<std::vector<AudioDevice>> try_get_audio_devices();
[[nodiscard]] core::Result<std::vector<DisplayOutput>> try_get_display_outputs();
[[nodiscard]] core::Result<std::vector<UsbDevice>> try_get_usb_devices();
[[nodiscard]] core::Result<std::vector<PciDevice>> try_get_pci_devices();
[[nodiscard]] core::Result<std::vector<SensorReading>> try_get_sensor_readings();

} // namespace hyggshios::system

namespace hyggshi = hyggshios;

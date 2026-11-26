#pragma once

#include <HAPI/core/result.hpp>

#include <string>
#include <string_view>

namespace hyggshios::system {

/// System board, chassis, and firmware (DMI/SMBIOS) hardware metadata.
struct HardwareInfo {
    std::string vendor;           ///< System manufacturer (DMI sys_vendor)
    std::string product_name;     ///< System model name (DMI product_name)
    std::string product_version;  ///< System model version (DMI product_version)
    std::string board_vendor;     ///< Motherboard manufacturer (DMI board_vendor)
    std::string board_name;       ///< Motherboard model name (DMI board_name)
    std::string bios_vendor;      ///< Firmware/BIOS vendor (DMI bios_vendor)
    std::string bios_version;     ///< Firmware/BIOS version (DMI bios_version)
    std::string chassis_type;     ///< Human-readable chassis type (e.g., "Notebook", "Desktop")
};

/// Converts an SMBIOS chassis type numeric code (e.g., "10") to a readable label.
[[nodiscard]] std::string decode_chassis_type(std::string_view raw_code);

/// Retrieves hardware DMI/SMBIOS information for the current host.
[[nodiscard]] core::Result<HardwareInfo> try_get_hardware_info();

/// Retrieves hardware DMI/SMBIOS information (best-effort on failure).
[[nodiscard]] HardwareInfo get_hardware_info();

}  // namespace hyggshios::system

namespace hyggshi = hyggshios;

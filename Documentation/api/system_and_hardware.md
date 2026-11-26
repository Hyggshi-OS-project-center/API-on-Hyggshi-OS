# System & Hardware API Reference

Include `<HAPI/system/system.hpp>` for all system headers, or include individual headers as needed.

## System & Runtime

- **`<HAPI/system/os_info.hpp>`**: `OsInfo`, `get_os_info()`, `try_get_os_info()`, `parse_os_release()`
- **`<HAPI/system/runtime_info.hpp>`**:
  - `try_get_uptime_info()` / `get_uptime_info()` (`/proc/uptime`)
  - `try_get_time_info()` / `get_time_info()` (UTC/local ISO-8601, timezone, offset)
  - `try_get_locale_info()` / `get_locale_info()` (`LANG`, `LC_ALL`, `LC_CTYPE`, `LC_TIME`)
  - `try_get_boot_info()` / `get_boot_info()` (`/proc/cmdline`, `/proc/stat` `btime`, `boot_id`)
  - `get_env(name)`, `list_env()`
- **`<HAPI/system/process_info.hpp>`**:
  - `try_get_process_info(pid)`, `try_get_current_process_info()`, `try_list_processes()`, `list_processes()`

## Hardware

- **`<HAPI/system/cpu_info.hpp>`**: `CpuInfo`, `try_get_cpu_info()`, `get_cpu_info()`, `parse_cpuinfo()`
- **`<HAPI/system/memory_info.hpp>`**: `MemoryInfo`, `try_get_memory_info()`, `get_memory_info()`, `parse_meminfo()`
- **`<HAPI/system/storage_info.hpp>`**: `StorageDevice`, `StorageInfo`, `try_get_storage_info(include_pseudo)`, `get_storage_info()`, `parse_proc_mounts()`
- **`<HAPI/system/hardware_info.hpp>`**: `HardwareInfo` (DMI/SMBIOS vendor, product, board, BIOS, chassis), `try_get_hardware_info()`, `decode_chassis_type()`
- **`<HAPI/system/hardware_devices.hpp>`**:
  - `try_get_gpu_devices()` (`/sys/class/drm`)
  - `try_get_batteries()` (`/sys/class/power_supply`)
  - `try_get_audio_devices()` (`/proc/asound/cards`)
  - `try_get_display_outputs()` (`/sys/class/drm`)
  - `try_get_usb_devices()` (`/sys/bus/usb/devices`)
  - `try_get_pci_devices()` (`/sys/bus/pci/devices`)
  - `try_get_sensor_readings()` (`/sys/class/hwmon`)

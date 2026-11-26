#pragma once

#include <HAPI/core/result.hpp>
#include <HAPI/system/cpu_info.hpp>
#include <HAPI/system/desktop_info.hpp>
#include <HAPI/system/hardware_info.hpp>
#include <HAPI/system/memory_info.hpp>
#include <HAPI/system/process_info.hpp>
#include <HAPI/system/storage_info.hpp>

#include <cstdint>
#include <vector>

namespace hyggshios::system::detail {

[[nodiscard]] core::Result<CpuInfo> query_linux_cpu_info();
[[nodiscard]] core::Result<MemoryInfo> query_linux_memory_info();
[[nodiscard]] core::Result<StorageInfo> query_linux_storage_info(bool include_pseudo);
[[nodiscard]] core::Result<ProcessInfo> query_linux_process_info(std::int32_t pid);
[[nodiscard]] core::Result<std::vector<ProcessInfo>> query_linux_process_list();
[[nodiscard]] core::Result<DesktopInfo> query_linux_desktop_info();
[[nodiscard]] core::Result<DesktopSettings> query_linux_desktop_settings();
[[nodiscard]] core::Result<HardwareInfo> query_linux_hardware_info();

} // namespace hyggshios::system::detail

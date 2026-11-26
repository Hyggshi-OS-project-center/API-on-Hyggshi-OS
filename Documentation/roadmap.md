# Roadmap

## Linux System & Platform APIs

| Milestone | Domain / API | Status |
| :--- | :--- | :--- |
| 01 | System OS, Kernel, Hostname (`os_info.hpp`) | Implemented |
| 02 | CPU Information (`cpu_info.hpp`) | Implemented |
| 03 | Memory Information (`memory_info.hpp`) | Implemented |
| 04 | Storage Mount & Disk Usage (`storage_info.hpp`) | Implemented |
| 05 | Process Information (`process_info.hpp`) | Implemented |
| 06 | Desktop Environment & Settings (`desktop_info.hpp`) | Implemented |
| 07 | Hardware DMI & Devices: GPU, Battery, Audio, Display, USB, PCI, Sensors (`hardware_info.hpp`, `hardware_devices.hpp`) | Implemented |
| 08 | Runtime: Uptime, Time, Locale, BootInfo, Environment (`runtime_info.hpp`) | Implemented |
| 09 | File API: Read, Write, Copy, Delete, Stat, Watch (`file_api.hpp`) | Implemented |
| 10 | Network API: Interfaces, IP, DNS, TCP Connections, Wi-Fi, Proxy, Stats (`network_api.hpp`) | Implemented |
| 11 | Software & Services: Installed Packages, Package Managers, Libraries, Systemd Services (`software_api.hpp`, `service_manager.hpp`) | Implemented |
| 12 | Security & Permissions: User, Groups, Capabilities, Session, Least-Privilege Policy (`security_api.hpp`, `permissions.hpp`) | Implemented |
| 13 | Core EventBus, Application Metadata & Desktop Notifications (`event.hpp`, `app_info.hpp`, `notification.hpp`) | Implemented |

## Bindings & Platform Adapters

| Component | Status |
| :--- | :--- |
| Public C ABI (`Bindings/c`, `libhapi_c.so`) | Implemented |
| Python SDK (`Bindings/python`, `ctypes`) | Implemented |
| LuaJIT SDK (`Bindings/lua`, `ffi`) | Implemented |
| TypeScript SDK (`Bindings/typescript` types) | Scaffolded |
| Windows Platform Adapter | Planned |
| Web / Emscripten Platform Adapter | Planned |

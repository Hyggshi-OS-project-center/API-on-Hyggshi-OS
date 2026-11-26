#include <HAPI/system/system.hpp>

#include <iomanip>
#include <iostream>

int main() {
    const auto os = hyggshi::system::get_os_info();
    const auto cpu = hyggshi::system::get_cpu_info();
    const auto mem = hyggshi::system::get_memory_info();
    const auto storage = hyggshi::system::get_storage_info();
    const auto self_proc = hyggshi::system::try_get_current_process_info();
    const auto desktop = hyggshi::system::get_desktop_info();
    const auto hw = hyggshi::system::get_hardware_info();

    std::cout << "=== OS Information ===\n"
              << "OS Name: " << os.name << " (" << os.version << ")\n"
              << "Kernel: " << os.kernel_name << ' ' << os.kernel_version << " [" << os.architecture
              << "]\n"
              << "Hostname: " << os.hostname << "\n\n";

    std::cout << "=== CPU Information ===\n"
              << "Model: " << cpu.model_name << '\n'
              << "Vendor: " << cpu.vendor_id << '\n'
              << "Cores: " << cpu.physical_cores << " physical / " << cpu.logical_cores
              << " logical\n"
              << "Frequency: " << std::fixed << std::setprecision(1) << cpu.frequency_mhz
              << " MHz\n\n";

    std::cout << "=== Memory Information ===\n"
              << "Total: " << (mem.total_kb / 1024) << " MiB\n"
              << "Available: " << (mem.available_kb / 1024) << " MiB\n"
              << "Used: " << (mem.used_kb / 1024) << " MiB (" << std::setprecision(1)
              << mem.usage_percent << "%)\n"
              << "Swap Used: " << (mem.swap_used_kb / 1024) << " / " << (mem.swap_total_kb / 1024)
              << " MiB\n\n";

    std::cout << "=== Storage Information ===\n";
    for (const auto& m : storage.mounts) {
        std::cout << m.mount_point << " (" << m.filesystem
                  << "): " << (m.used_bytes / (1024ULL * 1024ULL * 1024ULL)) << " / "
                  << (m.total_bytes / (1024ULL * 1024ULL * 1024ULL)) << " GiB ("
                  << std::setprecision(1) << m.usage_percent << "%)\n";
    }
    std::cout << '\n';

    if (self_proc.ok()) {
        const auto& p = self_proc.value();
        std::cout << "=== Current Process ===\n"
                  << "PID: " << p.pid << " | Name: " << p.name << " | Threads: " << p.threads
                  << " | RSS: " << p.memory_rss_kb << " KiB\n\n";
    }

    std::cout << "=== Desktop Environment ===\n"
              << "Desktop: "
              << (desktop.desktop_environment.empty() ? "N/A" : desktop.desktop_environment)
              << " | Session: " << desktop.session_type
              << " | Graphical: " << (desktop.is_graphical ? "Yes" : "No") << "\n\n";

    std::cout << "=== Hardware Information ===\n"
              << "Vendor: " << (hw.vendor.empty() ? "N/A" : hw.vendor) << '\n'
              << "Product: " << (hw.product_name.empty() ? "N/A" : hw.product_name) << '\n'
              << "Chassis: " << (hw.chassis_type.empty() ? "N/A" : hw.chassis_type) << '\n';

    return 0;
}

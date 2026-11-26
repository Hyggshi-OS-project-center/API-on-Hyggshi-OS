#include <HAPI/system/network_api.hpp>
#include <HAPI/system/software_api.hpp>
#include <HAPI/services/service_manager.hpp>
#include <iomanip>
#include <iostream>

int main() {
    // --- Network Interfaces ---
    std::cout << "=== Network Interfaces ===\n";
    if (auto ifaces = hyggshi::system::network::try_get_interfaces(); ifaces.ok()) {
        for (const auto& iface : ifaces.value()) {
            if (iface.is_loopback) continue;
            std::cout << (iface.is_up ? "[UP]  " : "[DOWN] ") << iface.name;
            if (!iface.mac_address.empty()) {
                std::cout << "  mac=" << iface.mac_address;
            }
            for (const auto& ip : iface.ipv4_addresses) {
                std::cout << "  ipv4=" << ip;
            }
            for (const auto& ip : iface.ipv6_addresses) {
                std::cout << "  ipv6=" << ip;
            }
            std::cout << "\n";
        }
    }

    // --- DNS ---
    std::cout << "\n=== DNS Configuration ===\n";
    if (auto dns = hyggshi::system::network::try_get_dns_config(); dns.ok()) {
        for (const auto& ns : dns.value().nameservers) {
            std::cout << "Nameserver: " << ns << "\n";
        }
        for (const auto& dom : dns.value().search_domains) {
            std::cout << "Search: " << dom << "\n";
        }
    }

    // --- Proxy ---
    std::cout << "\n=== Proxy ===\n";
    auto proxy = hyggshi::system::network::get_proxy_config();
    std::cout << "HTTP: " << (proxy.http_proxy.empty() ? "(none)" : proxy.http_proxy) << "\n";
    std::cout << "HTTPS: " << (proxy.https_proxy.empty() ? "(none)" : proxy.https_proxy) << "\n";

    // --- Network Statistics ---
    std::cout << "\n=== Interface Statistics ===\n";
    if (auto stats = hyggshi::system::network::try_get_statistics(); stats.ok()) {
        for (const auto& st : stats.value()) {
            if (st.rx_bytes == 0 && st.tx_bytes == 0) continue;
            std::cout << std::left << std::setw(10) << st.interface
                      << "  rx=" << (st.rx_bytes / 1024) << " KiB"
                      << "  tx=" << (st.tx_bytes / 1024) << " KiB\n";
        }
    }

    // --- TCP Connections (first 5) ---
    std::cout << "\n=== Active TCP Connections (first 5) ===\n";
    if (auto conns = hyggshi::system::network::try_get_connections(); conns.ok()) {
        int shown = 0;
        for (const auto& c : conns.value()) {
            if (c.state != "ESTABLISHED") continue;
            std::cout << c.local_address << ":" << c.local_port
                      << " -> " << c.remote_address << ":" << c.remote_port
                      << " [" << c.state << "]\n";
            if (++shown >= 5) break;
        }
        if (shown == 0) std::cout << "(no ESTABLISHED connections)\n";
    }

    // --- Package Manager & Installed Packages (first 10) ---
    std::cout << "\n=== Software ===\n";
    auto pm = hyggshi::system::software::get_package_manager_info();
    std::cout << "Primary package manager: " << pm.primary << "\n";
    std::cout << "Available:";
    for (const auto& m : pm.available) std::cout << " " << m;
    std::cout << "\n";

    if (auto pkgs = hyggshi::system::software::try_list_installed_packages(10); pkgs.ok()) {
        std::cout << "\nInstalled packages (first 10):\n";
        for (const auto& pkg : pkgs.value()) {
            std::cout << "  " << std::left << std::setw(30) << pkg.name << " " << pkg.version << "\n";
        }
    }

    // --- Systemd Services (first 5) ---
    std::cout << "\n=== Systemd Services (first 5) ===\n";
    if (auto svcs = hyggshi::services::try_list_services(5); svcs.ok()) {
        for (const auto& s : svcs.value()) {
            std::cout << std::left << std::setw(40) << s.name
                      << " " << std::setw(10) << s.active_state
                      << " " << s.sub_state << "\n";
        }
        if (svcs.value().empty()) std::cout << "(systemd not available)\n";
    }

    return 0;
}

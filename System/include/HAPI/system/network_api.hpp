#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace hyggshios::system::network {

/// Network interface and assigned IP addresses.
struct NetworkInterface {
    std::string name;
    std::string mac_address;
    std::vector<std::string> ipv4_addresses;
    std::vector<std::string> ipv6_addresses;
    bool is_up{false};
    bool is_loopback{false};
};

/// DNS resolver configuration from /etc/resolv.conf.
struct DnsConfig {
    std::vector<std::string> nameservers;
    std::vector<std::string> search_domains;
};

/// Active TCP/UDP socket summary from /proc/net/tcp.
struct NetworkConnection {
    std::string protocol;
    std::string local_address;
    std::uint16_t local_port{0};
    std::string remote_address;
    std::uint16_t remote_port{0};
    std::string state;
};

/// Wireless interface status.
struct WifiInfo {
    std::string interface;
    bool connected{false};
    std::int32_t signal_dbm{0};
};

/// Proxy environment configuration.
struct ProxyConfig {
    std::string http_proxy;
    std::string https_proxy;
    std::string no_proxy;
};

/// Per-interface traffic counters from /proc/net/dev.
struct InterfaceStats {
    std::string interface;
    std::uint64_t rx_bytes{0};
    std::uint64_t rx_packets{0};
    std::uint64_t tx_bytes{0};
    std::uint64_t tx_packets{0};
};

[[nodiscard]] core::Result<std::vector<NetworkInterface>> try_get_interfaces();
[[nodiscard]] core::Result<DnsConfig> try_get_dns_config();
[[nodiscard]] core::Result<std::vector<NetworkConnection>> try_get_connections();
[[nodiscard]] core::Result<std::vector<WifiInfo>> try_get_wifi_info();
[[nodiscard]] ProxyConfig get_proxy_config();
[[nodiscard]] core::Result<std::vector<InterfaceStats>> try_get_statistics();

} // namespace hyggshios::system::network

namespace hyggshi = hyggshios;

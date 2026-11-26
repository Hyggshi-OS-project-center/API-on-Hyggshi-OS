#include "linux_common.hpp"

#include <HAPI/core/permissions.hpp>
#include <HAPI/system/network_api.hpp>

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unordered_map>

namespace hyggshios::system::network {

namespace {

std::string get_env_str(const char* key) {
    const char* v = std::getenv(key);
    return v != nullptr ? std::string(v) : std::string{};
}

std::string decode_hex_ipv4(const std::string& hex_ip) {
    if (hex_ip.size() != 8) {
        return {};
    }
    unsigned int val = 0;
    if (std::sscanf(hex_ip.c_str(), "%x", &val) != 1) {
        return {};
    }
    struct in_addr addr{};
    addr.s_addr = val;
    char buf[INET_ADDRSTRLEN]{};
    if (::inet_ntop(AF_INET, &addr, buf, sizeof(buf)) != nullptr) {
        return std::string(buf);
    }
    return {};
}

std::string decode_tcp_state(const std::string& hex_state) {
    if (hex_state == "01") return "ESTABLISHED";
    if (hex_state == "02") return "SYN_SENT";
    if (hex_state == "03") return "SYN_RECV";
    if (hex_state == "04") return "FIN_WAIT1";
    if (hex_state == "05") return "FIN_WAIT2";
    if (hex_state == "06") return "TIME_WAIT";
    if (hex_state == "07") return "CLOSE";
    if (hex_state == "08") return "CLOSE_WAIT";
    if (hex_state == "09") return "LAST_ACK";
    if (hex_state == "0A") return "LISTEN";
    if (hex_state == "0B") return "CLOSING";
    return hex_state;
}

} // namespace

core::Result<std::vector<NetworkInterface>> try_get_interfaces() {
    if (!core::has_permission(core::Permission::NetworkInfoRead)) {
        return core::Result<std::vector<NetworkInterface>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: NetworkInfoRead is required",
            0,
        });
    }

    struct ifaddrs* ifaddr = nullptr;
    if (::getifaddrs(&ifaddr) != 0) {
        const int err = errno;
        return core::Result<std::vector<NetworkInterface>>::failure({
            core::ErrorCode::SystemCallFailed,
            "getifaddrs() failed: " + std::string(std::strerror(err)),
            err,
        });
    }

    std::unordered_map<std::string, NetworkInterface> by_name;
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == nullptr) {
            continue;
        }
        auto& iface = by_name[ifa->ifa_name];
        if (iface.name.empty()) {
            iface.name = ifa->ifa_name;
            iface.is_up = (ifa->ifa_flags & IFF_UP) != 0;
            iface.is_loopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;

            std::string mac;
            int mac_err = 0;
            const std::string mac_path = "/sys/class/net/" + iface.name + "/address";
            if (detail::read_file_text(mac_path.c_str(), mac, mac_err)) {
                iface.mac_address = std::string(detail::trim_ws(mac));
            }
        }

        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET) {
            char buf[INET_ADDRSTRLEN]{};
            const auto* sin = reinterpret_cast<const struct sockaddr_in*>(ifa->ifa_addr);
            if (::inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf)) != nullptr) {
                iface.ipv4_addresses.emplace_back(buf);
            }
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            char buf[INET6_ADDRSTRLEN]{};
            const auto* sin6 = reinterpret_cast<const struct sockaddr_in6*>(ifa->ifa_addr);
            if (::inet_ntop(AF_INET6, &sin6->sin6_addr, buf, sizeof(buf)) != nullptr) {
                iface.ipv6_addresses.emplace_back(buf);
            }
        }
    }
    ::freeifaddrs(ifaddr);

    std::vector<NetworkInterface> result;
    result.reserve(by_name.size());
    for (auto& [_, iface] : by_name) {
        result.push_back(std::move(iface));
    }
    return core::Result<std::vector<NetworkInterface>>::success(std::move(result));
}

core::Result<DnsConfig> try_get_dns_config() {
    if (!core::has_permission(core::Permission::NetworkInfoRead)) {
        return core::Result<DnsConfig>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: NetworkInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/etc/resolv.conf", content, err_no)) {
        return core::Result<DnsConfig>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /etc/resolv.conf",
            err_no,
        });
    }

    DnsConfig cfg;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        const std::string_view sv = detail::trim_ws(line);
        if (sv.empty() || sv.front() == '#') {
            continue;
        }
        std::istringstream lss{std::string(sv)};
        std::string key;
        lss >> key;
        if (key == "nameserver") {
            std::string ns;
            if (lss >> ns) {
                cfg.nameservers.push_back(std::move(ns));
            }
        } else if (key == "search" || key == "domain") {
            std::string dom;
            while (lss >> dom) {
                cfg.search_domains.push_back(std::move(dom));
            }
        }
    }
    return core::Result<DnsConfig>::success(std::move(cfg));
}

core::Result<std::vector<NetworkConnection>> try_get_connections() {
    if (!core::has_permission(core::Permission::NetworkInfoRead)) {
        return core::Result<std::vector<NetworkConnection>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: NetworkInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/net/tcp", content, err_no)) {
        return core::Result<std::vector<NetworkConnection>>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/net/tcp",
            err_no,
        });
    }

    std::vector<NetworkConnection> conns;
    std::istringstream iss(content);
    std::string line;
    std::getline(iss, line); // skip header
    while (std::getline(iss, line)) {
        std::istringstream lss(line);
        std::string sl;
        std::string local_ep;
        std::string rem_ep;
        std::string st;
        if (!(lss >> sl >> local_ep >> rem_ep >> st)) {
            continue;
        }
        const auto split_ep = [](const std::string& ep, std::string& ip, std::uint16_t& port) {
            const std::size_t colon = ep.find(':');
            if (colon == std::string::npos) {
                return;
            }
            ip = decode_hex_ipv4(ep.substr(0, colon));
            unsigned int p = 0;
            if (std::sscanf(ep.substr(colon + 1).c_str(), "%x", &p) == 1) {
                port = static_cast<std::uint16_t>(p);
            }
        };

        NetworkConnection conn;
        conn.protocol = "tcp";
        split_ep(local_ep, conn.local_address, conn.local_port);
        split_ep(rem_ep, conn.remote_address, conn.remote_port);
        conn.state = decode_tcp_state(st);
        conns.push_back(std::move(conn));
    }

    return core::Result<std::vector<NetworkConnection>>::success(std::move(conns));
}

core::Result<std::vector<WifiInfo>> try_get_wifi_info() {
    if (!core::has_permission(core::Permission::NetworkInfoRead)) {
        return core::Result<std::vector<WifiInfo>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: NetworkInfoRead is required",
            0,
        });
    }

    std::vector<WifiInfo> list;
    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/net/wireless", content, err_no)) {
        return core::Result<std::vector<WifiInfo>>::success(std::move(list));
    }

    std::istringstream iss(content);
    std::string line;
    std::getline(iss, line); // header 1
    std::getline(iss, line); // header 2
    while (std::getline(iss, line)) {
        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        WifiInfo wifi;
        wifi.interface = std::string(detail::trim_ws(line.substr(0, colon)));
        std::istringstream vals(line.substr(colon + 1));
        std::string status;
        double link = 0.0;
        double level = 0.0;
        if (vals >> status >> link >> level) {
            wifi.connected = link > 0.0;
            wifi.signal_dbm = static_cast<std::int32_t>(level);
        }
        list.push_back(std::move(wifi));
    }

    return core::Result<std::vector<WifiInfo>>::success(std::move(list));
}

ProxyConfig get_proxy_config() {
    ProxyConfig cfg;
    cfg.http_proxy = get_env_str("http_proxy");
    if (cfg.http_proxy.empty()) {
        cfg.http_proxy = get_env_str("HTTP_PROXY");
    }
    cfg.https_proxy = get_env_str("https_proxy");
    if (cfg.https_proxy.empty()) {
        cfg.https_proxy = get_env_str("HTTPS_PROXY");
    }
    cfg.no_proxy = get_env_str("no_proxy");
    if (cfg.no_proxy.empty()) {
        cfg.no_proxy = get_env_str("NO_PROXY");
    }
    return cfg;
}

core::Result<std::vector<InterfaceStats>> try_get_statistics() {
    if (!core::has_permission(core::Permission::NetworkInfoRead)) {
        return core::Result<std::vector<InterfaceStats>>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: NetworkInfoRead is required",
            0,
        });
    }

    std::string content;
    int err_no = 0;
    if (!detail::read_file_text("/proc/net/dev", content, err_no)) {
        return core::Result<std::vector<InterfaceStats>>::failure({
            core::ErrorCode::NotFound,
            "Failed to read /proc/net/dev",
            err_no,
        });
    }

    std::vector<InterfaceStats> stats;
    std::istringstream iss(content);
    std::string line;
    std::getline(iss, line); // header 1
    std::getline(iss, line); // header 2
    while (std::getline(iss, line)) {
        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        InterfaceStats st;
        st.interface = std::string(detail::trim_ws(line.substr(0, colon)));
        std::istringstream vals(line.substr(colon + 1));
        std::uint64_t dummy = 0;
        if (vals >> st.rx_bytes >> st.rx_packets >> dummy >> dummy >> dummy >> dummy >> dummy >>
            dummy >> st.tx_bytes >> st.tx_packets) {
            stats.push_back(std::move(st));
        }
    }

    return core::Result<std::vector<InterfaceStats>>::success(std::move(stats));
}

} // namespace hyggshios::system::network

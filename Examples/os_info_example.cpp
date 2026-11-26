#include <HAPI/system/os_info.hpp>

#include <iostream>

int main() {
    auto info = hyggshi::system::get_os_info();

    std::cout << "OS Name: " << info.name << '\n';
    std::cout << "OS Version: " << info.version << '\n';
    std::cout << "OS ID: " << info.id << '\n';
    std::cout << "OS Codename: " << info.codename << '\n';
    std::cout << "Pretty Name: " << info.pretty_name << '\n';
    std::cout << "Kernel: " << info.kernel_name << '\n';
    std::cout << "Kernel Version: " << info.kernel_version << '\n';
    std::cout << "Architecture: " << info.architecture << '\n';
    std::cout << "Hostname: " << info.hostname << '\n';

    return 0;
}

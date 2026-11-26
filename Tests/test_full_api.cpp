#include <HAPI/application/app_info.hpp>
#include <HAPI/application/notification.hpp>
#include <HAPI/core/event.hpp>
#include <HAPI/core/permissions.hpp>
#include <HAPI/core/platform.hpp>
#include <HAPI/services/service_manager.hpp>
#include <HAPI/system/system.hpp>

extern "C" {
#include <hapi_c.h>
}

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

void test_platform_and_events() {
    const auto plat = hyggshios::core::get_platform_info();
    assert(!plat.name.empty());
    assert(plat.abi_version == "0.1.0");

    int count = 0;
    std::string last_payload;
    const auto sub_id = hyggshios::core::EventBus::global().subscribe(
        "test.topic", [&](const hyggshios::core::Event& ev) {
            ++count;
            last_payload = ev.payload;
        });

    hyggshios::core::EventBus::global().emit(
        hyggshios::core::Event{"test.topic", "unit_test", "hello", 0});
    assert(count == 1);
    assert(last_payload == "hello");

    assert(hyggshios::core::EventBus::global().unsubscribe(sub_id));
    hyggshios::core::EventBus::global().emit(
        hyggshios::core::Event{"test.topic", "unit_test", "ignored", 0});
    assert(count == 1);
}

void test_runtime_and_security() {
    const auto up = hyggshios::system::try_get_uptime_info();
    assert(up.ok());
    assert(up.value().uptime_seconds > 0.0);

    const auto tm = hyggshios::system::try_get_time_info();
    assert(tm.ok());
    assert(tm.value().unix_timestamp_sec > 0);
    assert(!tm.value().iso8601_utc.empty());

    const auto loc = hyggshios::system::try_get_locale_info();
    assert(loc.ok());

    const auto boot = hyggshios::system::try_get_boot_info();
    assert(boot.ok());
    assert(boot.value().boot_time_unix > 0);

    const auto user = hyggshios::system::security::try_get_current_user();
    assert(user.ok());
    assert(!user.value().username.empty());

    const auto groups = hyggshios::system::security::try_get_current_groups();
    assert(groups.ok());

    const auto caps = hyggshios::system::security::try_get_capabilities();
    assert(caps.ok());
    assert(!caps.value().cap_effective.empty());

    const auto policy = hyggshios::system::security::get_security_policy();
    assert(!policy.allow_privilege_escalation);
}

void test_file_api_and_permissions() {
    const std::string tmp = "/tmp/hapi_file_api_test.txt";
    const std::string copy_tmp = "/tmp/hapi_file_api_test_copy.txt";

    auto w = hyggshios::system::file::write_text(tmp, "Hyggshi OS File API");
    assert(w.ok());
    assert(hyggshios::system::file::exists(tmp));

    auto r = hyggshios::system::file::read_text(tmp);
    assert(r.ok());
    assert(r.value() == "Hyggshi OS File API");

    auto st = hyggshios::system::file::stat(tmp);
    assert(st.ok());
    assert(st.value().is_regular_file);
    assert(st.value().size_bytes == 19);

    auto cp = hyggshios::system::file::copy(tmp, copy_tmp, true);
    assert(cp.ok());
    assert(hyggshios::system::file::exists(copy_tmp));

    // Verify least-privilege policy enforcement on FileWrite
    hyggshios::core::active_security_policy().allow_file_write = false;
    auto denied = hyggshios::system::file::write_text(tmp, "blocked");
    assert(!denied.ok());
    assert(denied.error().code == hyggshios::core::ErrorCode::PermissionDenied);
    hyggshios::core::active_security_policy().allow_file_write = true;

    assert(hyggshios::system::file::remove(tmp).ok());
    assert(hyggshios::system::file::remove(copy_tmp).ok());
}

void test_network_hardware_software_app_services() {
    const auto ifaces = hyggshios::system::network::try_get_interfaces();
    assert(ifaces.ok());
    assert(!ifaces.value().empty());

    const auto stats = hyggshios::system::network::try_get_statistics();
    assert(stats.ok());
    assert(!stats.value().empty());

    const auto dns = hyggshios::system::network::try_get_dns_config();
    assert(dns.ok());

    const auto conns = hyggshios::system::network::try_get_connections();
    assert(conns.ok());

    assert(hyggshios::system::try_get_gpu_devices().ok());
    assert(hyggshios::system::try_get_batteries().ok());
    assert(hyggshios::system::try_get_audio_devices().ok());
    assert(hyggshios::system::try_get_display_outputs().ok());
    assert(hyggshios::system::try_get_usb_devices().ok());
    assert(hyggshios::system::try_get_pci_devices().ok());
    assert(hyggshios::system::try_get_sensor_readings().ok());

    const auto pm = hyggshios::system::software::get_package_manager_info();
    assert(!pm.primary.empty());

    const auto libs = hyggshios::system::software::try_list_loaded_libraries();
    assert(libs.ok());
    assert(!libs.value().empty());

    const auto app = hyggshios::application::try_get_current_app();
    assert(app.ok());
    assert(app.value().pid > 0);
    assert(!app.value().executable_path.empty());

    const auto svcs = hyggshios::services::try_list_services(10);
    assert(svcs.ok());
}

void test_c_abi() {
    assert(std::string(hapi_version()) == "0.1.0");
    assert(std::string(hapi_platform_name()) == "linux");

    hapi_os_info_t os{};
    assert(hapi_get_os_info(&os) == HAPI_OK);
    assert(std::string(os.kernel_name) == "Linux");

    hapi_cpu_info_t cpu{};
    assert(hapi_get_cpu_info(&cpu) == HAPI_OK);
    assert(cpu.logical_cores > 0);

    hapi_memory_info_t mem{};
    assert(hapi_get_memory_info(&mem) == HAPI_OK);
    assert(mem.total_kb > 0);

    hapi_user_info_t user{};
    assert(hapi_get_user_info(&user) == HAPI_OK);
    assert(std::string(user.username).size() > 0);

    assert(hapi_get_os_info(nullptr) == HAPI_ERR_INVALID_ARG);
}

} // namespace

int main() {
    test_platform_and_events();
    test_runtime_and_security();
    test_file_api_and_permissions();
    test_network_hardware_software_app_services();
    test_c_abi();

    std::cout << "All HAPI full API & C ABI tests passed.\n";
    return 0;
}

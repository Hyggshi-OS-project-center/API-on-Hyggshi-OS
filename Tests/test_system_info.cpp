#include <HAPI/system/system.hpp>

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

void test_parse_cpuinfo() {
    constexpr const char* kCpuSample = R"(
processor	: 0
vendor_id	: GenuineIntel
model name	: Intel(R) Core(TM) i5-1135G7 @ 2.40GHz
cpu MHz		: 2400.000
cache size	: 8192 KB
physical id	: 0
core id		: 0
cpu cores	: 2

processor	: 1
vendor_id	: GenuineIntel
model name	: Intel(R) Core(TM) i5-1135G7 @ 2.40GHz
cpu MHz		: 2400.000
cache size	: 8192 KB
physical id	: 0
core id		: 0
cpu cores	: 2

processor	: 2
vendor_id	: GenuineIntel
model name	: Intel(R) Core(TM) i5-1135G7 @ 2.40GHz
cpu MHz		: 2400.000
cache size	: 8192 KB
physical id	: 0
core id		: 1
cpu cores	: 2
)";

    const auto cpu = hyggshios::system::parse_cpuinfo(kCpuSample);
    assert(cpu.vendor_id == "GenuineIntel");
    assert(cpu.model_name == "Intel(R) Core(TM) i5-1135G7 @ 2.40GHz");
    assert(cpu.logical_cores == 3);
    assert(cpu.physical_cores == 2);
    assert(cpu.cache_size_kb == 8192);
    assert(std::abs(cpu.frequency_mhz - 2400.0) < 0.01);
}

void test_parse_meminfo() {
    constexpr const char* kMemSample = R"(
MemTotal:       16384000 kB
MemFree:         2048000 kB
MemAvailable:    8192000 kB
Buffers:          512000 kB
Cached:          4096000 kB
SwapTotal:       2097152 kB
SwapFree:        1048576 kB
)";

    const auto mem = hyggshios::system::parse_meminfo(kMemSample);
    assert(mem.total_kb == 16384000);
    assert(mem.free_kb == 2048000);
    assert(mem.available_kb == 8192000);
    assert(mem.used_kb == 8192000);
    assert(mem.buffers_kb == 512000);
    assert(mem.cached_kb == 4096000);
    assert(mem.swap_total_kb == 2097152);
    assert(mem.swap_free_kb == 1048576);
    assert(mem.swap_used_kb == 1048576);
    assert(std::abs(mem.usage_percent - 50.0) < 0.01);
}

void test_parse_proc_mounts() {
    constexpr const char* kMountsSample = R"(
proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0
/dev/nvme0n1p2 / ext4 rw,relatime,errors=remount-ro 0 0
/dev/sda1 /mnt/My\040Drive vfat rw,relatime 0 0
tmpfs /run tmpfs rw,nosuid,nodev 0 0
)";

    const auto real_mounts = hyggshios::system::parse_proc_mounts(kMountsSample, false);
    assert(real_mounts.size() == 2);
    assert(real_mounts[0].device == "/dev/nvme0n1p2");
    assert(real_mounts[0].mount_point == "/");
    assert(real_mounts[0].filesystem == "ext4");
    assert(real_mounts[1].mount_point == "/mnt/My Drive");
    assert(real_mounts[1].filesystem == "vfat");

    const auto all_mounts = hyggshios::system::parse_proc_mounts(kMountsSample, true);
    assert(all_mounts.size() == 4);
}

void test_parse_proc_status() {
    constexpr const char* kStatusSample = R"(
Name:	hapi_tests
State:	R (running)
Pid:	1234
PPid:	1000
VmRSS:	   4096 kB
Threads:	4
)";

    const auto proc = hyggshios::system::parse_proc_status(1234, kStatusSample);
    assert(proc.pid == 1234);
    assert(proc.ppid == 1000);
    assert(proc.name == "hapi_tests");
    assert(proc.state == "R");
    assert(proc.memory_rss_kb == 4096);
    assert(proc.threads == 4);
}

void test_decode_chassis_type() {
    assert(hyggshios::system::decode_chassis_type("3") == "Desktop");
    assert(hyggshios::system::decode_chassis_type("10\n") == "Notebook");
    assert(hyggshios::system::decode_chassis_type("23") == "Rack Mount Chassis");
}

void test_live_linux_system_apis() {
#if defined(__linux__)
    const auto cpu_res = hyggshios::system::try_get_cpu_info();
    assert(cpu_res.ok());
    assert(cpu_res.value().logical_cores > 0);
    assert(cpu_res.value().physical_cores > 0);

    const auto mem_res = hyggshios::system::try_get_memory_info();
    assert(mem_res.ok());
    assert(mem_res.value().total_kb > 0);

    const auto storage_res = hyggshios::system::try_get_storage_info();
    assert(storage_res.ok());
    assert(!storage_res.value().mounts.empty());

    const auto self_proc = hyggshios::system::try_get_current_process_info();
    assert(self_proc.ok());
    assert(self_proc.value().pid > 0);
    assert(!self_proc.value().name.empty());

    const auto procs = hyggshios::system::try_list_processes();
    assert(procs.ok());
    assert(!procs.value().empty());

    const auto desktop_res = hyggshios::system::try_get_desktop_info();
    assert(desktop_res.ok());
    assert(!desktop_res.value().session_type.empty());

    const auto hw_res = hyggshios::system::try_get_hardware_info();
    assert(hw_res.ok());
#endif
}

}  // namespace

int main() {
    test_parse_cpuinfo();
    test_parse_meminfo();
    test_parse_proc_mounts();
    test_parse_proc_status();
    test_decode_chassis_type();
    test_live_linux_system_apis();

    std::cout << "All HAPI system info tests passed.\n";
    return 0;
}

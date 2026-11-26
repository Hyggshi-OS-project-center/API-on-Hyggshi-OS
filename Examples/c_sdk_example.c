#include <hapi_c.h>
#include <stdio.h>

int main(void) {
    printf("HAPI C SDK — version %s | platform: %s\n\n", hapi_version(), hapi_platform_name());

    /* OS Info */
    hapi_os_info_t os = {0};
    if (hapi_get_os_info(&os) == HAPI_OK) {
        printf("=== OS ===\n");
        printf("Name:    %s %s\n", os.name, os.version);
        printf("ID:      %s (%s)\n", os.id, os.codename);
        printf("Pretty:  %s\n", os.pretty_name);
        printf("Kernel:  %s %s [%s]\n", os.kernel_name, os.kernel_version, os.architecture);
        printf("Host:    %s\n\n", os.hostname);
    }

    /* CPU */
    hapi_cpu_info_t cpu = {0};
    if (hapi_get_cpu_info(&cpu) == HAPI_OK) {
        printf("=== CPU ===\n");
        printf("Model:   %s\n", cpu.model_name);
        printf("Vendor:  %s\n", cpu.vendor_id);
        printf("Cores:   %u physical / %u logical\n", cpu.physical_cores, cpu.logical_cores);
        printf("Freq:    %.1f MHz\n\n", cpu.frequency_mhz);
    }

    /* Memory */
    hapi_memory_info_t mem = {0};
    if (hapi_get_memory_info(&mem) == HAPI_OK) {
        printf("=== Memory ===\n");
        printf("Total:     %llu MiB\n", mem.total_kb / 1024);
        printf("Available: %llu MiB\n", mem.available_kb / 1024);
        printf("Used:      %llu MiB (%.1f%%)\n", mem.used_kb / 1024, mem.usage_percent);
        printf("Swap used: %llu / %llu MiB\n\n", mem.swap_used_kb / 1024, mem.swap_total_kb / 1024);
    }

    /* Uptime */
    hapi_uptime_info_t up = {0};
    if (hapi_get_uptime_info(&up) == HAPI_OK) {
        unsigned long h = (unsigned long)(up.uptime_seconds / 3600);
        unsigned long m = (unsigned long)((up.uptime_seconds - h * 3600) / 60);
        printf("=== Uptime ===\n");
        printf("Up: %luh %lum\n\n", h, m);
    }

    /* Desktop */
    hapi_desktop_info_t desk = {0};
    if (hapi_get_desktop_info(&desk) == HAPI_OK) {
        printf("=== Desktop ===\n");
        printf("DE:        %s\n", desk.desktop_environment[0] ? desk.desktop_environment : "N/A");
        printf("Session:   %s\n", desk.session_type);
        printf("Graphical: %s\n\n", desk.is_graphical ? "yes" : "no");
    }

    /* Hardware */
    hapi_hardware_info_t hw = {0};
    if (hapi_get_hardware_info(&hw) == HAPI_OK) {
        printf("=== Hardware ===\n");
        printf("Vendor:  %s\n", hw.vendor[0] ? hw.vendor : "N/A");
        printf("Product: %s\n", hw.product_name[0] ? hw.product_name : "N/A");
        printf("Chassis: %s\n\n", hw.chassis_type[0] ? hw.chassis_type : "N/A");
    }

    /* User */
    hapi_user_info_t user = {0};
    if (hapi_get_user_info(&user) == HAPI_OK) {
        printf("=== User ===\n");
        printf("Username: %s (uid=%u)\n", user.username, user.uid);
        printf("Home:     %s\n", user.home_dir);
        printf("Shell:    %s\n", user.shell);
        printf("Root:     %s\n\n", user.is_root ? "yes" : "no");
    }

    /* File API */
    printf("=== File API ===\n");
    const char* path = "/tmp/hapi_c_test.txt";
    if (hapi_file_write_text(path, "Hello from HAPI C SDK!\n") == HAPI_OK) {
        char buf[256] = {0};
        size_t out_len = 0;
        if (hapi_file_read_text(path, buf, sizeof(buf), &out_len) == HAPI_OK) {
            printf("Wrote and read back: %s", buf);
        }
        hapi_file_exists(path)
            ? printf("File exists: yes\n")
            : printf("File exists: no\n");
    }

    return 0;
}

#ifndef HAPI_C_H
#define HAPI_C_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define HAPI_C_EXPORT __declspec(dllexport)
#else
#define HAPI_C_EXPORT __attribute__((visibility("default")))
#endif

#define HAPI_OK 0
#define HAPI_ERR_INVALID_ARG -1
#define HAPI_ERR_NOT_FOUND -2
#define HAPI_ERR_PERMISSION -3
#define HAPI_ERR_IO -4
#define HAPI_ERR_UNSUPPORTED -5
#define HAPI_ERR_UNKNOWN -99

typedef struct hapi_os_info_t {
    char name[128];
    char version[64];
    char id[64];
    char codename[64];
    char pretty_name[256];
    char kernel_name[64];
    char kernel_version[128];
    char architecture[64];
    char hostname[256];
} hapi_os_info_t;

typedef struct hapi_cpu_info_t {
    char model_name[256];
    char vendor_id[64];
    uint32_t physical_cores;
    uint32_t logical_cores;
    double frequency_mhz;
    uint64_t cache_size_kb;
} hapi_cpu_info_t;

typedef struct hapi_memory_info_t {
    uint64_t total_kb;
    uint64_t free_kb;
    uint64_t available_kb;
    uint64_t used_kb;
    uint64_t buffers_kb;
    uint64_t cached_kb;
    uint64_t swap_total_kb;
    uint64_t swap_free_kb;
    uint64_t swap_used_kb;
    double usage_percent;
} hapi_memory_info_t;

typedef struct hapi_uptime_info_t {
    double uptime_seconds;
    double idle_seconds;
} hapi_uptime_info_t;

typedef struct hapi_desktop_info_t {
    char desktop_environment[128];
    char session_type[64];
    char display[128];
    char locale[64];
    int is_graphical;
} hapi_desktop_info_t;

typedef struct hapi_hardware_info_t {
    char vendor[128];
    char product_name[128];
    char product_version[64];
    char board_vendor[128];
    char board_name[128];
    char bios_vendor[128];
    char bios_version[64];
    char chassis_type[64];
} hapi_hardware_info_t;

typedef struct hapi_user_info_t {
    uint32_t uid;
    uint32_t euid;
    uint32_t gid;
    char username[128];
    char home_dir[512];
    char shell[256];
    int is_root;
} hapi_user_info_t;

HAPI_C_EXPORT const char* hapi_version(void);
HAPI_C_EXPORT const char* hapi_platform_name(void);

HAPI_C_EXPORT int hapi_get_os_info(hapi_os_info_t* out);
HAPI_C_EXPORT int hapi_get_cpu_info(hapi_cpu_info_t* out);
HAPI_C_EXPORT int hapi_get_memory_info(hapi_memory_info_t* out);
HAPI_C_EXPORT int hapi_get_uptime_info(hapi_uptime_info_t* out);
HAPI_C_EXPORT int hapi_get_desktop_info(hapi_desktop_info_t* out);
HAPI_C_EXPORT int hapi_get_hardware_info(hapi_hardware_info_t* out);
HAPI_C_EXPORT int hapi_get_user_info(hapi_user_info_t* out);

HAPI_C_EXPORT int hapi_file_exists(const char* path);
HAPI_C_EXPORT int hapi_file_read_text(const char* path,
                                      char* buf,
                                      size_t buf_size,
                                      size_t* out_len);
HAPI_C_EXPORT int hapi_file_write_text(const char* path, const char* content);

HAPI_C_EXPORT int hapi_send_notification(const char* app_name,
                                         const char* title,
                                         const char* body);

#ifdef __cplusplus
}
#endif

#endif /* HAPI_C_H */

cmake_minimum_required(VERSION 3.20)
project(API_on_Hyggshi_OS VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(HAPI_BUILD_EXAMPLES "Build HAPI example programs" ON)
option(HAPI_BUILD_TESTS "Build HAPI test suite" ON)

# Core library: common data types, errors, results, permissions, events, platform
add_library(hapi_core
    Core/src/error.cpp
    Core/src/event.cpp
    Core/src/platform.cpp
)
add_library(HAPI::core ALIAS hapi_core)

target_include_directories(hapi_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(hapi_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# System library: platform adapters (Linux first)
set(HAPI_SYSTEM_SOURCES
    System/src/os_info.cpp
    System/src/cpu_info.cpp
    System/src/memory_info.cpp
    System/src/storage_info.cpp
    System/src/process_info.cpp
    System/src/desktop_info.cpp
    System/src/hardware_info.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND HAPI_SYSTEM_SOURCES
        System/src/linux/linux_os_info.cpp
        System/src/linux/linux_cpu_info.cpp
        System/src/linux/linux_memory_info.cpp
        System/src/linux/linux_storage_info.cpp
        System/src/linux/linux_process_info.cpp
        System/src/linux/linux_desktop_info.cpp
        System/src/linux/linux_hardware_info.cpp
        System/src/linux/linux_runtime_info.cpp
        System/src/linux/linux_hardware_devices.cpp
        System/src/linux/linux_file_api.cpp
        System/src/linux/linux_network_api.cpp
        System/src/linux/linux_software_api.cpp
        System/src/linux/linux_security_api.cpp
    )
endif()

add_library(hapi_system ${HAPI_SYSTEM_SOURCES})
add_library(HAPI::system ALIAS hapi_system)

target_include_directories(hapi_system
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/System/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(hapi_system PUBLIC HAPI::core)

target_compile_options(hapi_system PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# Application library
add_library(hapi_application
    Application/src/app_info.cpp
    Application/src/notification.cpp
)
add_library(HAPI::application ALIAS hapi_application)

target_include_directories(hapi_application
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Application/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(hapi_application PUBLIC HAPI::core HAPI::system)

target_compile_options(hapi_application PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# Services library
add_library(hapi_services
    Services/src/service_manager.cpp
)
add_library(HAPI::services ALIAS hapi_services)

target_include_directories(hapi_services
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Services/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(hapi_services PUBLIC HAPI::core HAPI::system)

target_compile_options(hapi_services PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# Umbrella C++ target
add_library(hapi INTERFACE)
add_library(HAPI::HAPI ALIAS hapi)
target_link_libraries(hapi INTERFACE HAPI::core HAPI::system HAPI::application HAPI::services)

# Public C ABI shared library (used by C, Python, Lua, TypeScript/Node bindings)
add_library(hapi_c SHARED
    Bindings/c/src/hapi_c.cpp
)
target_include_directories(hapi_c
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bindings/c/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(hapi_c PRIVATE HAPI::HAPI)
target_compile_options(hapi_c PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

if(HAPI_BUILD_EXAMPLES)
    add_executable(os_info_example Examples/os_info_example.cpp)
    target_link_libraries(os_info_example PRIVATE HAPI::HAPI)

    add_executable(system_overview_example Examples/system_overview_example.cpp)
    target_link_libraries(system_overview_example PRIVATE HAPI::HAPI)

    add_executable(file_security_notification_example
        Examples/file_security_notification_example.cpp)
    target_link_libraries(file_security_notification_example PRIVATE HAPI::HAPI)

    add_executable(network_software_services_example
        Examples/network_software_services_example.cpp)
    target_link_libraries(network_software_services_example PRIVATE HAPI::HAPI)

    # C SDK example (compiled as C, linked to libhapi_c)
    add_executable(c_sdk_example Examples/c_sdk_example.c)
    set_source_files_properties(Examples/c_sdk_example.c PROPERTIES LANGUAGE C)
    target_include_directories(c_sdk_example PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/c/include)
    target_link_libraries(c_sdk_example PRIVATE hapi_c)
    # Ensure libhapi_c.so is found at runtime in the build dir
    set_target_properties(c_sdk_example PROPERTIES
        BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()

if(HAPI_BUILD_TESTS)
    enable_testing()

    add_executable(hapi_tests Tests/test_os_info.cpp)
    target_link_libraries(hapi_tests PRIVATE HAPI::HAPI)
    add_test(NAME HapiOsInfoTests COMMAND hapi_tests)

    add_executable(hapi_system_tests Tests/test_system_info.cpp)
    target_link_libraries(hapi_system_tests PRIVATE HAPI::HAPI)
    add_test(NAME HapiSystemInfoTests COMMAND hapi_system_tests)

    add_executable(hapi_full_tests Tests/test_full_api.cpp)
    target_include_directories(hapi_full_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/c/include)
    target_link_libraries(hapi_full_tests PRIVATE HAPI::HAPI hapi_c)
    add_test(NAME HapiFullApiTests COMMAND hapi_full_tests)
endif()

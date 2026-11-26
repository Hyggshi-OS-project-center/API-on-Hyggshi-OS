#include <HAPI/core/error.hpp>
#include <HAPI/core/result.hpp>
#include <HAPI/system/os_info.hpp>

#include <cassert>
#include <iostream>
#include <string>

namespace {

void test_parse_hyggshi_os_release() {
    constexpr const char* sample = R"osrel(
# Hyggshi OS release metadata
NAME="Hyggshi OS"
VERSION="2.0.0 (pyramid)"
ID=hyggshi
VERSION_ID="2.0.0"
VERSION_CODENAME=pyramid
PRETTY_NAME="Hyggshi OS 2.0.0"
)osrel";

    const auto info = hyggshios::system::parse_os_release(sample);
    assert(info.name == "Hyggshi OS");
    assert(info.version == "2.0.0");
    assert(info.id == "hyggshi");
    assert(info.codename == "pyramid");
    assert(info.pretty_name == "Hyggshi OS 2.0.0");
}

void test_parse_quotes_escapes_and_fallbacks() {
    constexpr const char* sample = R"(
NAME='Custom \"Linux\"'
VERSION="1.0-lts"
ID=custom
UBUNTU_CODENAME=noble
PRETTY_NAME="Custom \"OS\" \\ \$ \`"
)";

    const auto info = hyggshi::system::parse_os_release(sample);
    assert(info.name == R"(Custom \"Linux\")");
    assert(info.version == "1.0-lts");
    assert(info.id == "custom");
    assert(info.codename == "noble");
    assert(info.pretty_name == "Custom \"OS\" \\ $ `");
}

void test_core_result_and_error() {
    const auto ok_res = hyggshios::core::Result<int>::success(42);
    assert(ok_res.ok());
    assert(ok_res.has_value());
    assert(ok_res.value() == 42);
    assert(ok_res.value_or(0) == 42);

    const auto err_res = hyggshios::core::Result<int>::failure(hyggshios::core::Error{
        hyggshios::core::ErrorCode::NotFound,
        "missing resource",
        2,
    });
    assert(!err_res.ok());
    assert(err_res.error().code == hyggshios::core::ErrorCode::NotFound);
    assert(err_res.error().message == "missing resource");
    assert(err_res.error().os_errno == 2);
    assert(err_res.value_or(99) == 99);
    assert(hyggshios::core::to_string(hyggshios::core::ErrorCode::NotFound) == "NotFound");
}

void test_live_linux_os_info() {
#ifdef __linux__
    const auto res = hyggshios::system::try_get_os_info();
    assert(res.ok());

    const auto& info = res.value();
    assert(!info.kernel_name.empty());
    assert(!info.kernel_version.empty());
    assert(!info.architecture.empty());
    assert(!info.hostname.empty());
    assert(!info.name.empty() || !info.pretty_name.empty());

    const auto direct = hyggshi::system::get_os_info();
    assert(direct.kernel_name == info.kernel_name);
    assert(direct.kernel_version == info.kernel_version);
    assert(direct.architecture == info.architecture);
    assert(direct.hostname == info.hostname);
#endif
}

} // namespace

int main() {
    test_parse_hyggshi_os_release();
    test_parse_quotes_escapes_and_fallbacks();
    test_core_result_and_error();
    test_live_linux_os_info();

    std::cout << "All HAPI OS Info tests passed.\n";
    return 0;
}

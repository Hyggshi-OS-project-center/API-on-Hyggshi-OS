#pragma once

#include <string>
#include <string_view>

namespace hyggshios::core {

/// Categorized error codes returned by Hyggshi OS API operations.
enum class ErrorCode {
    None = 0,
    NotFound,
    PermissionDenied,
    IoError,
    ParseError,
    SystemCallFailed,
    UnsupportedPlatform,
    InvalidArgument,
    Unknown
};

/// Returns a human-readable label for an ErrorCode.
std::string_view to_string(ErrorCode code) noexcept;

/// Structured error value carrying a code, diagnostic message, and optional OS errno.
struct Error {
    ErrorCode code{ErrorCode::None};
    std::string message;
    int os_errno{0};

    [[nodiscard]] bool is_ok() const noexcept { return code == ErrorCode::None; }
    explicit operator bool() const noexcept { return !is_ok(); }
};

} // namespace hyggshios::core

namespace hyggshi = hyggshios;

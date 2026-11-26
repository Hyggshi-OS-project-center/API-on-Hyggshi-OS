#include <HAPI/core/error.hpp>

namespace hyggshios::core {

std::string_view to_string(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::None:
            return "None";
        case ErrorCode::NotFound:
            return "NotFound";
        case ErrorCode::PermissionDenied:
            return "PermissionDenied";
        case ErrorCode::IoError:
            return "IoError";
        case ErrorCode::ParseError:
            return "ParseError";
        case ErrorCode::SystemCallFailed:
            return "SystemCallFailed";
        case ErrorCode::UnsupportedPlatform:
            return "UnsupportedPlatform";
        case ErrorCode::InvalidArgument:
            return "InvalidArgument";
        case ErrorCode::Unknown:
            return "Unknown";
    }
    return "Unknown";
}

}  // namespace hyggshios::core

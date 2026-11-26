#pragma once

#include <cstdint>
#include <string_view>

#define HAPI_VERSION_MAJOR 0
#define HAPI_VERSION_MINOR 1
#define HAPI_VERSION_PATCH 0

namespace hyggshios::core {

/// Semantic version string of the Hyggshi OS API library.
inline constexpr std::string_view kApiVersion = "0.1.0";

} // namespace hyggshios::core

namespace hyggshi = hyggshios;

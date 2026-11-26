#pragma once

#include <HAPI/core/result.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace hyggshios::system::file {

/// File metadata returned by stat().
struct FileStat {
    std::uint64_t size_bytes{0};
    std::int64_t modified_unix{0};
    bool is_directory{false};
    bool is_regular_file{false};
    bool is_symlink{false};
};

/// File change event kinds emitted by watch().
enum class WatchEventKind {
    Modified,
    Deleted,
    Created
};

using WatchCallback = std::function<void(const std::string& path, WatchEventKind kind)>;

[[nodiscard]] core::Result<std::string> read_text(const std::string& path);
[[nodiscard]] core::Result<std::vector<std::uint8_t>> read_bytes(const std::string& path);

[[nodiscard]] core::Result<bool> write_text(const std::string& path, const std::string& content);
[[nodiscard]] core::Result<bool> write_bytes(const std::string& path,
                                             const std::vector<std::uint8_t>& data);

[[nodiscard]] core::Result<bool> copy(const std::string& src_path,
                                      const std::string& dst_path,
                                      bool overwrite = true);
[[nodiscard]] core::Result<bool> remove(const std::string& path);
[[nodiscard]] bool exists(const std::string& path);
[[nodiscard]] core::Result<FileStat> stat(const std::string& path);

/// Polls a file path for changes up to max_checks iterations (interval_ms apart) and invokes
/// callback + emits "file.changed" on core::EventBus::global().
[[nodiscard]] core::Result<std::uint32_t> watch_once(const std::string& path,
                                                     std::uint64_t last_mtime_ns,
                                                     WatchCallback callback);

} // namespace hyggshios::system::file

namespace hyggshi = hyggshios;

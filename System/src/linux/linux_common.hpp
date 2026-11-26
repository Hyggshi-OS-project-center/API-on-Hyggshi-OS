#pragma once

#include <cerrno>
#include <charconv>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

namespace hyggshios::system::detail {

inline std::string_view trim_ws(std::string_view sv) noexcept {
    while (!sv.empty() &&
           (sv.front() == ' ' || sv.front() == '\t' || sv.front() == '\r' || sv.front() == '\n')) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() &&
           (sv.back() == ' ' || sv.back() == '\t' || sv.back() == '\r' || sv.back() == '\n')) {
        sv.remove_suffix(1);
    }
    return sv;
}

inline bool read_file_text(const char* path, std::string& out, int& err_no) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        err_no = errno;
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (file.bad()) {
        err_no = errno;
        return false;
    }
    out = buffer.str();
    return true;
}

template <typename IntT>
inline bool parse_int(std::string_view sv, IntT& out) noexcept {
    sv = trim_ws(sv);
    if (sv.empty()) {
        return false;
    }
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return ec == std::errc{} && ptr == sv.data() + sv.size();
}

template <typename IntT>
inline bool parse_leading_int(std::string_view sv, IntT& out) noexcept {
    sv = trim_ws(sv);
    if (sv.empty()) {
        return false;
    }
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return ec == std::errc{};
}

}  // namespace hyggshios::system::detail

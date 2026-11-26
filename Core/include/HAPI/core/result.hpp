#pragma once

#include <HAPI/core/error.hpp>

#include <stdexcept>
#include <utility>
#include <variant>

namespace hyggshios::core {

/// Lightweight result container holding either a value of type T or an Error.
template <typename T>
class Result {
   public:
    static Result success(T value) { return Result(std::move(value)); }
    static Result failure(Error error) { return Result(std::move(error)); }

    [[nodiscard]] bool ok() const noexcept { return std::holds_alternative<T>(data_); }
    [[nodiscard]] bool has_value() const noexcept { return ok(); }
    explicit operator bool() const noexcept { return ok(); }

    [[nodiscard]] const T& value() const& {
        if (!ok()) {
            throw std::runtime_error(error().message);
        }
        return std::get<T>(data_);
    }

    [[nodiscard]] T& value() & {
        if (!ok()) {
            throw std::runtime_error(error().message);
        }
        return std::get<T>(data_);
    }

    [[nodiscard]] T&& value() && {
        if (!ok()) {
            throw std::runtime_error(error().message);
        }
        return std::get<T>(std::move(data_));
    }

    [[nodiscard]] const Error& error() const& {
        if (ok()) {
            static const Error kNoError{};
            return kNoError;
        }
        return std::get<Error>(data_);
    }

    [[nodiscard]] T value_or(T fallback) const& {
        return ok() ? std::get<T>(data_) : std::move(fallback);
    }

   private:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(Error error) : data_(std::move(error)) {}

    std::variant<T, Error> data_;
};

}  // namespace hyggshios::core

namespace hyggshi = hyggshios;

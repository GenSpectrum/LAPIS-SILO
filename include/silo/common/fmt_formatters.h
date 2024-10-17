#pragma once

#include <filesystem>
#include <optional>

#include <fmt/format.h>
#include <chrono>

template <typename T>
struct [[maybe_unused]] fmt::formatter<std::optional<T>> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const std::optional<T>& val, format_context& ctx)
      -> decltype(ctx.out()) {
      if (val.has_value()) {
         return fmt::format_to(ctx.out(), "'{}'", val.value());
      }
      return fmt::format_to(ctx.out(), "null");
   }
};

template <>
struct [[maybe_unused]] fmt::formatter<std::filesystem::path> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const std::filesystem::path& val, format_context& ctx)
      -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", val.string());
   }
};

namespace silo::common {

std::string toIsoString(
   const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& time_point
);

}  // namespace silo::common

template <>
struct [[maybe_unused]] fmt::formatter<
   std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>
    : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& val,
      format_context& ctx
   ) -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", silo::common::toIsoString(val));
   }
};

#pragma once

#include <chrono>
#include <filesystem>
#include <optional>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"

template <typename T, typename U>
struct [[maybe_unused]] fmt::formatter<std::unordered_map<T, U>> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const std::unordered_map<T, U>& val, format_context& ctx)
      -> decltype(ctx.out()) {
      auto out = ctx.out();
      fmt::format_to(out, "{{\n");
      for (const auto& [key, value] : val) {
         fmt::format_to(out, "  {}: {},\n", key, value);
      }
      fmt::format_to(out, "}}");
      return out;
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

}

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

namespace fmt {

template <>
struct formatter<nlohmann::json> {
   constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

   template <typename FormatContext>
   auto format(const nlohmann::json& json, FormatContext& ctx) -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", json.dump());
   }
};

template <>
struct formatter<YAML::Node> {
   constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

   template <typename FormatContext>
   auto format(const YAML::Node& yaml, FormatContext& ctx) -> decltype(ctx.out()) {
      YAML::Emitter out;
      out << yaml;
      SILO_ASSERT(out.good());
      return fmt::format_to(ctx.out(), "{}", out.c_str());
   }
};

}  // namespace fmt

namespace silo::common {

inline std::string toDebugString(const std::string& value) {
   return fmt::format("'{}'", value);
}

inline std::string toDebugString(const std::filesystem::path& value) {
   return fmt::format("'{}'", value);
}

inline std::string toDebugString(bool value) {
   return fmt::format("{}", value);
}

inline std::string toDebugString(int32_t value) {
   return fmt::format("{}", value);
}

inline std::string toDebugString(uint32_t value) {
   return fmt::format("{}", value);
}

inline std::string toDebugString(size_t value) {
   return fmt::format("{}", value);
}

inline std::string toDebugString(
   std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> value
) {
   return fmt::format("{}", value);
}

template <typename T>
std::string toDebugString(const std::optional<T>& value) {
   if (value.has_value()) {
      return fmt::format("{}", toDebugString(value.value()));
   }
   return fmt::format("null");
}

}  // namespace silo::common

#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

#include "silo/common/fmt_formatters.h"

namespace nlohmann {

// NOLINTNEXTLINE(readability-identifier-naming)
inline void to_json(
   nlohmann::json& js_object,
   const std::optional<std::filesystem::path>& opt_path
) {
   if (opt_path.has_value()) {
      js_object = opt_path.value();
   } else {
      js_object = nullptr;
   }
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline void from_json(
   const nlohmann::json& js_object,
   std::optional<std::filesystem::path>& opt_path
) {
   if (!js_object) {
      opt_path = std::nullopt;
   } else {
      std::filesystem::path path = js_object;
      opt_path = path;
   }
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline void to_json(nlohmann::json& js_object, const std::optional<std::string>& opt_string) {
   if (opt_string.has_value()) {
      js_object = opt_string.value();
   } else {
      js_object = nullptr;
   }
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline void from_json(const nlohmann::json& js_object, std::optional<std::string>& opt_string) {
   if (!js_object) {
      opt_string = std::nullopt;
   } else {
      std::string string = js_object;
      opt_string = string;
   }
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline void to_json(nlohmann::json& js_object, const std::optional<uint32_t>& opt_uint) {
   if (opt_uint.has_value()) {
      js_object = opt_uint.value();
   } else {
      js_object = nullptr;
   }
}
// NOLINTNEXTLINE(readability-identifier-naming)
inline void from_json(const nlohmann::json& js_object, std::optional<uint32_t>& opt_uint) {
   if (!js_object) {
      opt_uint = std::nullopt;
   } else {
      uint32_t uint = js_object;
      opt_uint = uint;
   }
}

// NOLINTNEXTLINE(readability-identifier-naming)
inline void to_json(
   nlohmann::json& js_object,
   const std::optional<
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>& opt_nanos
) {
   if (opt_nanos.has_value()) {
      js_object = silo::common::toIsoString(opt_nanos.value());
   } else {
      js_object = nullptr;
   }
}
// NOLINTNEXTLINE(readability-identifier-naming)
inline void from_json(
   const nlohmann::json& js_object,
   std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>&
      opt_nanos
) {
   SILO_UNIMPLEMENTED();
}

}  // namespace nlohmann
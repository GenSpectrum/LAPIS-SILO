#pragma once

#include <filesystem>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <utility>

#include <rhydb/common/data_version.h>
#include <rhydb/common/fmt_formatters.h>

namespace rhydb {

class InvalidRhyDBDataSourceException : public std::runtime_error {
  public:
   explicit InvalidRhyDBDataSourceException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit InvalidRhyDBDataSourceException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

class RhyDBDataSource {
   RhyDBDataSource(std::filesystem::path path, rhydb::DataVersion data_version)
       : path(std::move(path)),
         data_version(std::move(data_version)) {}

  public:
   RhyDBDataSource() = delete;

   std::filesystem::path path;
   rhydb::DataVersion data_version;

   static RhyDBDataSource checkValidDataSource(
      const std::filesystem::path& candidate_data_source_path
   );

   [[nodiscard]] std::string toDebugString() const;
};

class RhyDBDirectory {
   std::filesystem::path directory;

  public:
   explicit RhyDBDirectory(std::filesystem::path directory)
       : directory(std::move(directory)) {}

   [[nodiscard]] std::optional<RhyDBDataSource> getMostRecentDataDirectory() const;

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(RhyDBDirectory, directory);
};

}  // namespace rhydb

template <>
struct [[maybe_unused]] fmt::formatter<rhydb::RhyDBDirectory> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const rhydb::RhyDBDirectory& val, format_context& ctx)
      -> decltype(ctx.out()) {
      auto out = ctx.out();
      const nlohmann::json json = val;
      fmt::format_to(out, "{}", json.dump());
      return out;
   }
};

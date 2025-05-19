#pragma once

#include <filesystem>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <silo/common/data_version.h>
#include <silo/common/fmt_formatters.h>

namespace silo {

class InvalidSiloDataSourceException : public std::runtime_error {
  public:
   explicit InvalidSiloDataSourceException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit InvalidSiloDataSourceException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

class SiloDataSource {
   SiloDataSource() = delete;

   SiloDataSource(std::filesystem::path path, silo::DataVersion data_version)
       : path(path),
         data_version(data_version) {}

  public:
   std::filesystem::path path;
   silo::DataVersion data_version;

   static SiloDataSource checkValidDataSource(
      const std::filesystem::path& candidate_data_source_path
   );
};

class SiloDirectory {
   std::filesystem::path directory;

  public:
   explicit SiloDirectory(std::filesystem::path directory)
       : directory(std::move(directory)) {}

   std::optional<SiloDataSource> getMostRecentDataDirectory() const;

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(SiloDirectory, directory);
};

}  // namespace silo

template <>
struct [[maybe_unused]] fmt::formatter<silo::SiloDirectory> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const silo::SiloDirectory& val, format_context& ctx)
      -> decltype(ctx.out()) {
      auto out = ctx.out();
      nlohmann::json json = val;
      fmt::format_to(out, "{}", json.dump());
      return out;
   }
};

#pragma once

#include <filesystem>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <silo/common/fmt_formatters.h>
#include <silo/common/data_version.h>

namespace silo{

class SiloDataSource{
   SiloDataSource() = delete;

   SiloDataSource(std::filesystem::path path, silo::DataVersion data_version) : path(path),
         data_version(data_version) {}

  public:
   std::filesystem::path path;
   silo::DataVersion data_version;

   static std::optional<SiloDataSource> checkValidDataSource(const std::filesystem::path& candidate_data_source_path);
};

class SiloDirectory {
   std::filesystem::path directory;

  public:
   explicit SiloDirectory(std::filesystem::path directory) : directory(std::move(directory)) {}

   std::optional<SiloDataSource> getMostRecentDataDirectory() const;

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(SiloDirectory, directory);
};

}

template<>
struct [[maybe_unused]] fmt::formatter<silo::SiloDirectory> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const silo::SiloDirectory& val, format_context& ctx)
      -> decltype(ctx.out()) {
      auto out = ctx.out();
      nlohmann::json json = val;
      fmt::format_to(out, "{}", json);
      return out;
   }
};

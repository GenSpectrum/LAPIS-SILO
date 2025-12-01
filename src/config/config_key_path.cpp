#include "config/config_key_path.h"

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>

namespace {
bool isLowerCaseOrNumeric(char character) {
   return (std::islower(character) != 0 || std::isdigit(character) != 0);
}
}  // namespace

namespace silo::config {

std::vector<std::vector<std::string>> ConfigKeyPath::getPath() const {
   return path;
}

std::optional<ConfigKeyPath> ConfigKeyPath::tryFrom(
   const std::vector<std::vector<std::string>>& paths
) {
   if (paths.empty()) {
      return std::nullopt;
   }
   for (const auto& sublevel : paths) {
      if (sublevel.empty()) {
         return std::nullopt;
      }
      for (const std::string& string : sublevel) {
         if (string.empty()) {
            return std::nullopt;
         }
         if (!std::ranges::all_of(string, isLowerCaseOrNumeric)) {
            return std::nullopt;
         }
      }
   }
   ConfigKeyPath result;
   result.path = paths;
   return result;
}

AmbiguousConfigKeyPath AmbiguousConfigKeyPath::from(const ConfigKeyPath& key_path) {
   AmbiguousConfigKeyPath result;
   for (const auto& sublevel : key_path.getPath()) {
      std::ranges::copy(sublevel, std::back_inserter(result.path));
   }
   return result;
}

std::optional<AmbiguousConfigKeyPath> AmbiguousConfigKeyPath::tryFrom(
   std::vector<std::string>&& path
) {
   auto arbitrary_representation = ConfigKeyPath::tryFrom({path});
   if (arbitrary_representation.has_value()) {
      return AmbiguousConfigKeyPath::from(arbitrary_representation.value());
   }
   return std::nullopt;
}

}  // namespace silo::config

std::size_t std::hash<silo::config::ConfigKeyPath>::operator()(
   const silo::config::ConfigKeyPath& key
) const {
   std::size_t seed = 0;
   for (const auto& segment : key.getPath()) {
      boost::hash_combine(seed, segment);
   }
   return seed;
}

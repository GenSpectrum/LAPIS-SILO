#include "silo/common/file_to_string.h"

#include <fstream>
#include <sstream>

namespace silo::common {

std::optional<std::string> fileToString(const std::filesystem::path& path) {
   std::ifstream file(path, std::ios::in | std::ios::binary);
   if (!file) {
      return std::nullopt;
   }
   std::ostringstream contents;
   if (file.peek() != std::ifstream::traits_type::eof()) {
      contents << file.rdbuf();
      if (contents.fail()) {
         return std::nullopt;
      }
   }
   return contents.str();
}

}  // namespace silo::common
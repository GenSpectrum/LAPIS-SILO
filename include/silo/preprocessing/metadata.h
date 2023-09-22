#pragma once

#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

#include <duckdb.hpp>

namespace silo::preprocessing {

class MetadataReader {
  public:
   struct Iterator {
      using iterator_category = std::forward_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = int;
      using pointer = value_type*;
      using reference = value_type&;
   };

   std::unique_ptr<duckdb::MaterializedQueryResult> file_content;

   explicit MetadataReader(const std::filesystem::path& metadata_path);

   std::vector<std::string> getColumn(const std::string& column_name);
};

}  // namespace silo::preprocessing

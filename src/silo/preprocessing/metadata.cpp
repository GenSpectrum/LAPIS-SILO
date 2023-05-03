#include "silo/preprocessing/metadata.h"

#include <csv.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

std::vector<std::string> MetadataReader::getColumn(
   const std::filesystem::path& metadata_path,
   const std::string& column_name
) {
   auto reader = MetadataReader::getReader(metadata_path);

   try {
      std::vector<std::string> column;
      for (const auto& row : reader) {
         column.push_back(row[column_name].get());
      }
      return column;
   } catch (const std::exception& exception) {
      const std::basic_string<char, std::char_traits<char>, std::allocator<char>>& message =
         "Failed to read metadata file '" + metadata_path.string() + "': " + exception.what();
      throw PreprocessingException(message);
   }
}

csv::CSVReader MetadataReader::getReader(const std::filesystem::path& metadata_path) {
   try {
      return {metadata_path.string()};
   } catch (const std::exception& exception) {
      const std::basic_string<char, std::char_traits<char>, std::allocator<char>>& message =
         "Failed to read metadata file '" + metadata_path.string() + "': " + exception.what();
      throw PreprocessingException(message);
   }
}


}  // namespace silo::preprocessing
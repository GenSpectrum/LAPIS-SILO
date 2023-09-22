#include "silo/preprocessing/metadata.h"

#include <chrono>
#include <exception>
#include <ostream>
#include <string>
#include <thread>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

MetadataReader::MetadataReader(const std::filesystem::path& metadata_path) {
   duckdb::DuckDB db;
   duckdb::Connection connection(db);
   file_content = connection.Query(fmt::format("SELECT * FROM '{}'", metadata_path.string()));
}

std::vector<std::string> MetadataReader::getColumn(const std::string& column_name) {
   auto column_pointer =
      std::find(file_content->names.begin(), file_content->names.end(), column_name);
   if (column_pointer == file_content->names.end()) {
      const std::string message =
         "Failed to read metadata column '" + column_name + "'. Not found in metadata.";
      throw PreprocessingException(message);
   }
   size_t column_index = std::distance(file_content->names.begin(), column_pointer);

   std::vector<std::string> column;
   for (auto it = file_content->begin(); it != file_content->end(); ++it) {
      const std::string string_value =
         duckdb::StringValue::Get(it.current_row.GetValue<duckdb::Value>(column_index)
         );  // TODO validate type
      column.emplace_back(string_value);
   }
   return column;
}

}  // namespace silo::preprocessing
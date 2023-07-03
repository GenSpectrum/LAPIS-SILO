#include "silo/preprocessing/metadata.h"

#include <boost/algorithm/string/join.hpp>
#include <csv.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

MetadataReader::MetadataReader(const std::filesystem::path& metadata_path) try
    : reader(metadata_path.string()) {
} catch (const std::exception& exception) {
   const std::string message =
      "Failed to read metadata file '" + metadata_path.string() + "': " + exception.what();
   throw PreprocessingException(message);
}

std::vector<std::string> MetadataReader::getColumn(const std::string& column_name) {
   if (reader.index_of(column_name) == csv::CSV_NOT_FOUND) {
      const std::string message = "Failed to read metadata column '" + column_name + "'";
      throw PreprocessingException(message);
   }
   std::vector<std::string> column;
   for (const auto& row : reader) {
      column.push_back(row[column_name].get());
   }
   return column;
}

MetadataWriter::MetadataWriter(const std::filesystem::path& metadata_path)
    : out_stream(metadata_path){};

void MetadataWriter::writeHeader(const csv::CSVReader& csv_reader) {
   const auto header = boost::algorithm::join(csv_reader.get_col_names(), "\t");
   out_stream << header << '\n';
}

void MetadataWriter::writeRow(const csv::CSVRow& row) {
   const auto& row_string =
      boost::algorithm::join(static_cast<std::vector<std::string>>(row), "\t");
   out_stream << row_string << '\n';
}

}  // namespace silo::preprocessing
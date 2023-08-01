#include "silo/preprocessing/metadata.h"

#include <exception>
#include <ostream>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace {

csv::CSVReader buildReader(const std::filesystem::path& metadata_path) {
   SPDLOG_INFO("Reading metadata file: {}", metadata_path.string());
   try {
      csv::CSVFormat format;
      format.delimiter('\t');
      format.variable_columns(csv::VariableColumnPolicy::THROW);
      format.header_row(0);
      return csv::CSVReader{metadata_path.string(), format};
   } catch (const std::exception& exception) {
      const std::string message =
         "Failed to read metadata file '" + metadata_path.string() + "': " + exception.what();
      throw silo::PreprocessingException(message);
   }
}
}  // namespace

namespace silo::preprocessing {

MetadataReader::MetadataReader(const std::filesystem::path& metadata_path)
    : reader(buildReader(metadata_path)) {}

std::vector<std::string> MetadataReader::getColumn(const std::string& column_name) {
   if (reader.index_of(column_name) == csv::CSV_NOT_FOUND) {
      const std::string message = "Failed to read metadata column '" + column_name + "'";
      throw PreprocessingException(message);
   }
   std::vector<std::string> column;
   csv::CSVRow row;
   while (reader.read_row(row)) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(2));
      column.emplace_back(row[column_name].get_sv());
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
#include "silo/preprocessing/metadata.h"

#include <boost/algorithm/string/join.hpp>
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

MetadataWriter::MetadataWriter(std::unique_ptr<std::ostream> out_stream)
    : out_stream(std::move(out_stream)){};

void MetadataWriter::writeHeader(const csv::CSVReader& csv_reader) {
   const auto header = boost::algorithm::join(csv_reader.get_col_names(), "\t");
   *out_stream << header << '\n';
}

void MetadataWriter::writeRow(const csv::CSVRow& row) {
   const auto& row_string =
      boost::algorithm::join(static_cast<std::vector<std::string>>(row), "\t");
   *out_stream << row_string << '\n';
}

}  // namespace silo::preprocessing
#ifndef SILO_SRC_SILO_STORAGE_CSV_READER_H_
#define SILO_SRC_SILO_STORAGE_CSV_READER_H_

#include <filesystem>
#include <string>
#include <vector>

#include <csv.hpp>

namespace silo::preprocessing {

class MetadataReader {
  public:
   csv::CSVReader reader;

   explicit MetadataReader(const std::filesystem::path& metadata_path);

   std::vector<std::string> getColumn(const std::string& column_name);
};

class MetadataWriter {
  private:
   std::ofstream out_stream;

  public:
   explicit MetadataWriter(const std::filesystem::path& metadata_path);

   void writeHeader(const csv::CSVReader& csv_reader);

   void writeRow(const csv::CSVRow& row);
};

}  // namespace silo::preprocessing

#endif  // SILO_SRC_SILO_STORAGE_CSV_READER_H_

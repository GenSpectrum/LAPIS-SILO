#ifndef SILO_SRC_SILO_STORAGE_CSV_READER_H_
#define SILO_SRC_SILO_STORAGE_CSV_READER_H_

#include <filesystem>
#include <string>
#include <vector>

#include <csv.hpp>

namespace silo::preprocessing {

const std::string PRIMARY_KEY = "gisaid_epi_isl";
const std::string PANGO_LINEAGE = "pango_lineage";

class MetadataReader {
  public:
   static std::vector<std::string> getColumn(
      const std::filesystem::path& metadata_path,
      const std::string& column_name
   );

   static csv::CSVReader getReader(const std::filesystem::path& metadata_path);
};

}  // namespace silo::preprocessing

#endif  // SILO_SRC_SILO_STORAGE_CSV_READER_H_

#ifndef SILO_SRC_SILO_STORAGE_CSV_READER_H_
#define SILO_SRC_SILO_STORAGE_CSV_READER_H_

#include <filesystem>
#include <string>
#include <vector>

namespace silo::preprocessing {

class MetadataReader {
  public:
   static std::vector<std::string> getColumn(
      const std::filesystem::path& metadata_path,
      const std::string& column_name
   );
};

}  // namespace silo::preprocessing

#endif  // SILO_SRC_SILO_STORAGE_CSV_READER_H_

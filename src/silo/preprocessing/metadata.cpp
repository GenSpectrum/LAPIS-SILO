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

}  // namespace silo::preprocessing
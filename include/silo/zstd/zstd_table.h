#pragma once

#include <string>

#include <silo/file_reader/file_reader.h>

namespace duckdb {
struct Connection;
}

namespace silo {
class FastaReader;

class ZstdTable {
   duckdb::Connection& connection;
   std::string table_name;
   std::string_view compression_dict;

   ZstdTable(
      duckdb::Connection& connection,
      std::string table_name,
      std::string_view compression_dict
   );

  public:
   static ZstdTable generate(
      duckdb::Connection& connection,
      const std::string& table_name,
      FileReader& file_reader,
      std::string_view reference_sequence
   );
};

}  // namespace silo

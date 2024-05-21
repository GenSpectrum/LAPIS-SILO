#pragma once

#include <string>

#include <silo/file_reader/file_reader.h>

namespace duckdb {
struct Connection;
}

namespace silo {
class ZstdFastaReader;
class ZstdFastaTableReader;
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
   ZstdFastaTableReader getReader(std::string_view where_clause, std::string_view order_by_clause);

   static ZstdTable generate(
      duckdb::Connection& connection,
      const std::string& table_name,
      ZstdFastaReader& file_reader,
      std::string_view reference_sequence
   );

   static ZstdTable generate(
      duckdb::Connection& connection,
      const std::string& table_name,
      FileReader& file_reader,
      std::string_view reference_sequence
   );
};

}  // namespace silo

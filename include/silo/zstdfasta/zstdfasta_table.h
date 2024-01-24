#pragma once

#include <string>

namespace duckdb {
struct Connection;
}

namespace silo {
class ZstdFastaReader;
class ZstdFastaTableReader;
class FastaReader;

class ZstdFastaTable {
   duckdb::Connection& connection;
   std::string table_name;
   std::string_view compression_dict;

   ZstdFastaTable(
      duckdb::Connection& connection,
      std::string table_name,
      std::string_view compression_dict
   );

  public:
   ZstdFastaTableReader getReader(std::string_view where_clause, std::string_view order_by_clause);

   static ZstdFastaTable generate(
      duckdb::Connection& connection,
      const std::string& table_name,
      ZstdFastaReader& file_reader,
      std::string_view reference_sequence
   );

   static ZstdFastaTable generate(
      duckdb::Connection& connection,
      const std::string& table_name,
      FastaReader& file_reader,
      std::string_view reference_sequence
   );
};

}  // namespace silo

#pragma once

#include <string>

namespace duckdb {
struct Connection;
}

namespace silo {
class ZstdFastaReader;
class FastaReader;

class ZstdFastaTable {
  public:
   static void generate(
      duckdb::Connection& connection,
      std::string table_name,
      ZstdFastaReader& file_reader
   );

   static void generate(
      duckdb::Connection& connection,
      std::string table_name,
      FastaReader& file_reader,
      std::string_view reference_genome
   );
};

}  // namespace silo

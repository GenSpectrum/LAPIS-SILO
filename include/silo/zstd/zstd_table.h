#pragma once

#include <string>

#include "silo/sequence_file_reader/sequence_file_reader.h"

namespace duckdb {
struct Connection;
}

namespace silo {

class ZstdTable {
   duckdb::Connection& connection;
   std::string table_name;

   ZstdTable(duckdb::Connection& connection, std::string table_name)
       : connection(connection),
         table_name(std::move(table_name)){};

  public:
   static ZstdTable generate(
      duckdb::Connection& connection,
      const std::string& table_name,
      sequence_file_reader::SequenceFileReader& file_reader,
      std::string_view reference_sequence
   );
};

}  // namespace silo

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include <oneapi/tbb/enumerable_thread_specific.h>
#include <duckdb.hpp>

#include "silo/storage/pango_lineage_alias.h"
#include "silo/zstdfasta/zstd_compressor.h"

namespace silo {

class ZstdCompressor;

class CustomSqlFunction {
  public:
   explicit CustomSqlFunction(std::string function_name);

   virtual void addToConnection(duckdb::Connection& connection) = 0;

  protected:
   std::string function_name;
};

class CompressSequence : public CustomSqlFunction {
   std::shared_ptr<silo::ZstdCDictionary> zstd_dictionary;
   tbb::enumerable_thread_specific<silo::ZstdCompressor> compressor;

  public:
   CompressSequence(
      std::string_view symbol_type_name,
      std::string_view sequence_name,
      std::string_view reference
   );

   void addToConnection(duckdb::Connection& connection) override;

   std::string generateSqlStatement(const std::string& column_name_in_data) const;
};

}  // namespace silo
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include <oneapi/tbb/enumerable_thread_specific.h>
#include <duckdb.hpp>

#include "silo/preprocessing/identifier.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/zstd/zstd_compressor.h"

namespace silo {

class ZstdCompressor;

class CustomSqlFunction {
  public:
   explicit CustomSqlFunction(preprocessing::Identifier function_name_);

   virtual void addToConnection(duckdb::Connection& connection) = 0;

  protected:
   preprocessing::Identifier function_name;
};

class CompressSequence : public CustomSqlFunction {
   std::shared_ptr<silo::ZstdCDictionary> zstd_dictionary;
   tbb::enumerable_thread_specific<silo::ZstdCompressor> compressor;

  public:
   CompressSequence(preprocessing::Identifier function_name, std::string_view reference);

   void addToConnection(duckdb::Connection& connection) override;

   std::string generateSqlStatement(const std::string& column_name_in_data) const;
};

}  // namespace silo
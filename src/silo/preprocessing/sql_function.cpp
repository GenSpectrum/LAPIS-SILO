#include "silo/preprocessing/sql_function.h"

#include <spdlog/spdlog.h>

#include "silo/common/pango_lineage.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstdfasta/zstd_compressor.h"

using duckdb::Connection;
using duckdb::DataChunk;
using duckdb::ExpressionState;
using duckdb::LogicalType;
using duckdb::string_t;
using duckdb::StringVector;
using duckdb::UnaryExecutor;
using duckdb::Vector;

silo::CustomSqlFunction::CustomSqlFunction(std::string function_name)
    : function_name(std::move(function_name)) {}

silo::CompressSequence::CompressSequence(
   std::string_view symbol_type_name,
   std::string_view sequence_name,
   std::string_view reference
)
    : CustomSqlFunction(fmt::format("compress_{}_{}", symbol_type_name, sequence_name)),
      zstd_dictionary(std::make_shared<ZstdCDictionary>(reference, 2)),
      compressor(tbb::enumerable_thread_specific<ZstdCompressor>([&]() {
         return ZstdCompressor(zstd_dictionary);
      })) {
   SPDLOG_DEBUG(
      "CompressSequence UDF {} - initialize compressor for sequence  for '{}'",
      function_name,
      sequence_name
   );
}

void silo::CompressSequence::addToConnection(Connection& connection) {
   const std::function<void(DataChunk&, ExpressionState&, Vector&)> compressor_wrapper =
      [&](DataChunk& args, ExpressionState& /*state*/, Vector& result) {
         UnaryExecutor::Execute<string_t, string_t>(
            args.data[0],
            result,
            args.size(),
            [&](const string_t uncompressed) {
               silo::ZstdCompressor& local_compressor = compressor.local();
               const std::string_view compressed =
                  local_compressor.compress(uncompressed.GetData(), uncompressed.GetSize());

               return StringVector::AddStringOrBlob(
                  result, compressed.data(), static_cast<uint32_t>(compressed.size())
               );
            }
         );
      };

   connection.CreateVectorizedFunction(
      function_name, {LogicalType::VARCHAR}, LogicalType::BLOB, compressor_wrapper
   );
}
std::string silo::CompressSequence::generateSqlStatement(const std::string& column_name_in_data
) const {
   return fmt::format("{0}({1})", function_name, column_name_in_data);
}

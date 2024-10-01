#include "silo/preprocessing/sql_function.h"

#include <spdlog/spdlog.h>

#include "silo/preprocessing/identifiers.h"
#include "silo/zstd/zstd_compressor.h"

using duckdb::Connection;
using duckdb::DataChunk;
using duckdb::ExpressionState;
using duckdb::LogicalType;
using duckdb::string_t;
using duckdb::StringVector;
using duckdb::UnaryExecutor;
using duckdb::Vector;

silo::CustomSqlFunction::CustomSqlFunction(preprocessing::Identifier function_name_)
    : function_name(std::move(function_name_)) {
   SPDLOG_DEBUG(
      "Registering UDF {} (escaped in SQL strings as: {})",
      function_name.getRawIdentifier(),
      function_name.escape()
   );
}

silo::CompressSequence::CompressSequence(
   preprocessing::Identifier function_name,
   std::string_view reference
)
    : CustomSqlFunction(std::move(function_name)),
      zstd_dictionary(std::make_shared<ZstdCDictionary>(reference, 2)),
      compressor(tbb::enumerable_thread_specific<ZstdCompressor>([&]() {
         return ZstdCompressor(zstd_dictionary);
      })) {}

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
      function_name.getRawIdentifier(),
      {LogicalType::VARCHAR},
      LogicalType::BLOB,
      compressor_wrapper
   );
}
std::string silo::CompressSequence::generateSqlStatement(const std::string& column_name_in_data
) const {
   return fmt::format("{0}({1})", function_name.escape(), column_name_in_data);
}

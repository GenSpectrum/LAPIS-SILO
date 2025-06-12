#include "silo/query_engine/actions/fasta.h"

#include <memory>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/numbers.h"
#include "silo/common/panic.h"
#include "silo/common/range.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"

using silo::common::add1;
using silo::common::Range;

namespace silo::query_engine::actions {

Fasta::Fasta(std::vector<std::string>&& sequence_names)
    : sequence_names(sequence_names) {}

std::vector<schema::ColumnIdentifier> Fasta::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields;
   auto columns_in_database =
      table_schema.getColumnByType<storage::column::ZstdCompressedStringColumnPartition>();
   for (const auto& sequence_name : sequence_names) {
      schema::ColumnIdentifier column_identifier{
         sequence_name, schema::ColumnType::ZSTD_COMPRESSED_STRING
      };
      CHECK_SILO_QUERY(
         std::ranges::find(columns_in_database, column_identifier) != columns_in_database.end(),
         "Database does not contain an unaligned sequence with name: '" + sequence_name + "'"
      )
      fields.emplace_back(column_identifier);
   }
   fields.push_back(table_schema.primary_key);
   return fields;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceNames") && json["sequenceNames"].is_array(),
      "The Fasta action requires a sequenceNames field, which must be an array of strings"
   );
   std::vector<std::string> sequence_names;
   for (const auto& child : json["sequenceNames"]) {
      CHECK_SILO_QUERY(
         child.is_string(),
         "The Fasta action requires a sequenceNames field, which must be an array of strings; "
         "while parsing array encountered the element " +
            child.dump() + " which is not of type string"
      );
      sequence_names.emplace_back(child.get<std::string>());
   }
   action = std::make_unique<Fasta>(std::move(sequence_names));
}

}  // namespace silo::query_engine::actions

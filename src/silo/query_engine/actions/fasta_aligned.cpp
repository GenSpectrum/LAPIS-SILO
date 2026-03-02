#include "silo/query_engine/actions/fasta_aligned.h"

#include <optional>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::actions {

FastaAligned::FastaAligned(
   std::vector<std::string>&& sequence_names,
   std::vector<std::string>&& additional_fields
)
    : sequence_names(sequence_names),
      additional_fields(additional_fields) {}

std::vector<schema::ColumnIdentifier> FastaAligned::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   std::set<schema::ColumnIdentifier> fields;
   for (const auto& sequence_name : sequence_names) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value() && isSequenceColumn(column.value().type),
         "The table does not contain the SequenceColumn '{}'",
         sequence_name
      );
      fields.emplace(column.value());
   }
   for (const auto& additional_field : additional_fields) {
      auto column = table_schema.getColumn(additional_field);
      CHECK_SILO_QUERY(
         column.has_value(), "The table does not contain the Column '{}'", additional_field
      );
      fields.emplace(column.value());
   }
   fields.emplace(table_schema.primary_key);
   std::vector<schema::ColumnIdentifier> unique_fields{fields.begin(), fields.end()};
   return unique_fields;
}

}  // namespace silo::query_engine::actions

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/array/builder_base.h>
#include <arrow/array/builder_binary.h>
#include <arrow/array/builder_primitive.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/common/symbol_map.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

template <typename SymbolType>
class Mutations : public Action {
  public:
   static constexpr std::string_view MUTATION_FIELD_NAME = "mutation";
   static constexpr std::string_view MUTATION_FROM_FIELD_NAME = "mutationFrom";
   static constexpr std::string_view MUTATION_TO_FIELD_NAME = "mutationTo";
   static constexpr std::string_view POSITION_FIELD_NAME = "position";
   static constexpr std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   static constexpr std::string_view PROPORTION_FIELD_NAME = "proportion";
   static constexpr std::string_view COVERAGE_FIELD_NAME = "coverage";
   static constexpr std::string_view COUNT_FIELD_NAME = "count";
   static constexpr std::array<std::string_view, 8> VALID_FIELDS{
      MUTATION_FIELD_NAME,
      MUTATION_FROM_FIELD_NAME,
      MUTATION_TO_FIELD_NAME,
      POSITION_FIELD_NAME,
      SEQUENCE_FIELD_NAME,
      PROPORTION_FIELD_NAME,
      COVERAGE_FIELD_NAME,
      COUNT_FIELD_NAME
   };

  private:
   std::vector<std::string> sequence_names;
   double min_proportion;
   std::vector<std::string_view> fields;

   struct PrefilteredBitmaps {
      std::vector<std::pair<
         const CopyOnWriteBitmap&,
         const storage::column::SequenceColumnPartition<SymbolType>&>>
         bitmaps;
      std::vector<std::pair<size_t, const storage::column::SequenceColumnPartition<SymbolType>&>>
         full_bitmaps;
   };

   static std::unordered_map<std::string, Mutations<SymbolType>::PrefilteredBitmaps>
   preFilterBitmaps(
      const silo::storage::Table& table,
      std::vector<CopyOnWriteBitmap>& bitmap_filter
   );

   static void addPositionToMutationCountsForMixedBitmaps(
      uint32_t position_idx,
      const PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static void addPositionToMutationCountsForFullBitmaps(
      uint32_t position_idx,
      const PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static SymbolMap<SymbolType, std::vector<uint32_t>> calculateMutationsPerPosition(
      const storage::column::SequenceColumnMetadata<SymbolType>& sequence_store,
      const PrefilteredBitmaps& bitmap_filter
   );

   static arrow::Status addMutationsToOutput(
      const std::string& sequence_name,
      const storage::column::SequenceColumnMetadata<SymbolType>& sequence_store,
      double min_proportion,
      const PrefilteredBitmaps& bitmap_filter,
      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder
   );

   void validateOrderByFields(const schema::TableSchema& schema) const override;

   QueryResult execute(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const override {
      SILO_PANIC("Legacy execute called on already migrated action. Programming error.");
   }

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
      const config::QueryOptions& query_options
   ) override;

  public:
   explicit Mutations(
      std::vector<std::string>&& aa_sequence_names,
      double min_proportion,
      std::vector<std::string_view>&& fields
   );

   std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Mutations<SymbolType>>& action);

}  // namespace silo::query_engine::actions

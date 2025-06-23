#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/result.h>
#include <arrow/status.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

template <typename SymbolType>
class InsertionAggregation : public Action {
   static constexpr std::string_view POSITION_FIELD_NAME = "position";
   static constexpr std::string_view INSERTED_SYMBOLS_FIELD_NAME = "insertedSymbols";
   static constexpr std::string_view INSERTION_FIELD_NAME = "insertion";
   static constexpr std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   static constexpr std::string_view COUNT_FIELD_NAME = "count";

   std::vector<std::string> sequence_names;

   struct PrefilteredBitmaps {
      std::vector<std::pair<
         const CopyOnWriteBitmap&,
         const silo::storage::insertion::InsertionIndex<SymbolType>&>>
         bitmaps;
      std::vector<std::pair<size_t, const silo::storage::insertion::InsertionIndex<SymbolType>&>>
         full_bitmaps;
   };

   static arrow::Status addAggregatedInsertionsToInsertionCounts(
      const std::string& sequence_name,
      bool show_sequence_in_response,
      const PrefilteredBitmaps& prefiltered_bitmaps,
      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder
   );

   std::
      unordered_map<std::string, InsertionAggregation<SymbolType>::PrefilteredBitmaps> static preFilterBitmaps(
         std::shared_ptr<const storage::Table> table,
         const std::vector<std::string>& sequence_names,
         std::vector<CopyOnWriteBitmap>& bitmap_filter
      );

  public:
   InsertionAggregation(std::vector<std::string>&& sequence_names);

   void validateOrderByFields(const schema::TableSchema& schema) const override;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
      const config::QueryOptions& query_options
   ) const override;

   std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<SymbolType>>& action
);

}  // namespace silo::query_engine::actions

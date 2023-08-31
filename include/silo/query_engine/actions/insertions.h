#pragma once

#include "silo/query_engine/actions/action.h"
#include "silo/storage/column/insertion_column.h"
#include "silo/storage/column/insertion_index.h"

namespace silo::query_engine {

struct QueryResultEntry;

namespace actions {

template <typename SymbolType>
class InsertionAggregation : public Action {
   static constexpr std::string_view POSITION_FIELD_NAME = "position";
   static constexpr std::string_view INSERTION_FIELD_NAME = "insertions";
   static constexpr std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   static constexpr std::string_view COUNT_FIELD_NAME = "count";

   std::vector<std::string> column_names;
   std::vector<std::string> sequence_names;

   struct PrefilteredBitmaps {
      std::vector<std::pair<
         const OperatorResult&,
         const silo::storage::column::insertion::InsertionIndex<SymbolType>&>>
         bitmaps;
      std::vector<std::pair<
         const OperatorResult&,
         const silo::storage::column::insertion::InsertionIndex<SymbolType>&>>
         full_bitmaps;
   };

   void addAllColumnIndexesToPreFilteredBitmaps(
      const silo::storage::column::InsertionColumnPartition<SymbolType>& column,
      const OperatorResult& filter,
      std::unordered_map<std::string, InsertionAggregation<SymbolType>::PrefilteredBitmaps>&
         bitmaps_to_evaluate
   ) const;

   void addAggregatedInsertionsToInsertionCounts(
      std::vector<QueryResultEntry>& output,
      const std::string& sequence_name,
      const PrefilteredBitmaps& prefiltered_bitmaps
   ) const;

   std::unordered_map<std::string, InsertionAggregation<SymbolType>::PrefilteredBitmaps>
   validateFieldsAndPreFilterBitmaps(
      const Database& database,
      std::vector<OperatorResult>& bitmap_filter
   ) const;

  public:
   InsertionAggregation(
      std::vector<std::string>&& column,
      std::vector<std::string>&& sequence_names
   );

   void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<SymbolType>>& action
);

}  // namespace actions
}  // namespace silo::query_engine

#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"

namespace silo {
class Database;
namespace storage {
namespace insertion {
template <typename SymbolType>
class InsertionIndex;
}  // namespace insertion
}  // namespace storage
}  // namespace silo

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
         const OperatorResult&,
         const silo::storage::insertion::InsertionIndex<SymbolType>&>>
         bitmaps;
      std::vector<std::pair<
         const OperatorResult&,
         const silo::storage::insertion::InsertionIndex<SymbolType>&>>
         full_bitmaps;
   };

   void addAggregatedInsertionsToInsertionCounts(
      std::vector<QueryResultEntry>& output,
      const std::string& sequence_name,
      bool show_sequence_in_response,
      const PrefilteredBitmaps& prefiltered_bitmaps
   ) const;

   std::unordered_map<std::string, InsertionAggregation<SymbolType>::PrefilteredBitmaps>
   validateFieldsAndPreFilterBitmaps(
      const Database& database,
      std::vector<OperatorResult>& bitmap_filter
   ) const;

  public:
   InsertionAggregation(std::vector<std::string>&& sequence_names);

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

}  // namespace silo::query_engine::actions

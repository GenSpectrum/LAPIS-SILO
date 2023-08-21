#ifndef SILO_INSERTIONS_H
#define SILO_INSERTIONS_H

#include "silo/query_engine/actions/action.h"
#include "silo/storage/column/insertion_index.h"

namespace silo::query_engine {

struct QueryResultEntry;

namespace actions {

template <typename Symbol>
class InsertionAggregation : public Action {
   static constexpr std::string_view POSITION_FIELD_NAME = "position";
   static constexpr std::string_view INSERTION_FIELD_NAME = "insertions";
   static constexpr std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   static constexpr std::string_view COUNT_FIELD_NAME = "count";

   std::string column_name;
   std::vector<std::string> sequence_names;

   struct PrefilteredBitmaps {
      std::vector<std::pair<
         const OperatorResult&,
         const silo::storage::column::insertion::InsertionIndex<Symbol>&>>
         bitmaps;
      std::vector<std::pair<
         const OperatorResult&,
         const silo::storage::column::insertion::InsertionIndex<Symbol>&>>
         full_bitmaps;
   };

   void addAggregatedInsertionsToInsertionCounts(
      std::vector<QueryResultEntry>& output,
      const std::string& sequence_name,
      const PrefilteredBitmaps& prefiltered_bitmaps
   ) const;

   std::unordered_map<std::string, InsertionAggregation<Symbol>::PrefilteredBitmaps>
   validateFieldsAndPreFilterBitmaps(
      const Database& database,
      std::vector<OperatorResult>& bitmap_filter
   ) const;

  public:
   InsertionAggregation(std::string column, std::vector<std::string>&& sequence_names);

   void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;
};

template <typename Symbol>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionAggregation<Symbol>>& action);

}  // namespace actions
}  // namespace silo::query_engine

#endif  // SILO_INSERTIONS_H

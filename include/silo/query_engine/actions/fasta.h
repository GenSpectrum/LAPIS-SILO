#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"

namespace silo {
namespace query_engine {
struct OperatorResult;
}  // namespace query_engine
struct Database;
struct DatabasePartition;
}  // namespace silo

namespace silo::query_engine::actions {

class Fasta : public Action {
   static constexpr size_t SEQUENCE_LIMIT = 10'000;

   std::vector<std::string> sequence_names;

   [[nodiscard]] void validateOrderByFields(const Database& database) const override;

   QueryResult execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
      const override;

   void addSequencesToResultsForPartition(
      QueryResult& results,
      const silo::DatabasePartition& database_partition,
      const OperatorResult& bitmap,
      const std::string& primary_key_column
   ) const;

  public:
   explicit Fasta(std::vector<std::string>&& sequence_names);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action);

}  // namespace silo::query_engine::actions

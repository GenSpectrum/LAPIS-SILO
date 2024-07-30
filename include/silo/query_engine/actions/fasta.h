#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"

namespace silo {
namespace query_engine {
class OperatorResult;
}  // namespace query_engine
class Database;
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine::actions {

class Fasta : public Action {
   static constexpr size_t SEQUENCE_LIMIT = 10'000;

   std::vector<std::string> sequence_names;

   void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;

  public:
   explicit Fasta(std::vector<std::string>&& sequence_names);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action);

}  // namespace silo::query_engine::actions

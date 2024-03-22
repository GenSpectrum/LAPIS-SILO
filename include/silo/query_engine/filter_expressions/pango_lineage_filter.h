#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
class Database;
class DatabasePartition;

namespace query_engine {
namespace operators {
class Operator;
}  // namespace operators
}  // namespace query_engine
}  // namespace silo

namespace silo::query_engine::filter_expressions {

class PangoLineageFilter : public Expression {
   std::string column;
   std::string lineage;
   bool include_sublineages;

  public:
   explicit PangoLineageFilter(
      std::string column,
      std::string lineage_key,
      bool include_sublineages
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PangoLineageFilter>& filter);

}  // namespace silo::query_engine::filter_expressions

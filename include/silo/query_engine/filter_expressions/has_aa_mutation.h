#ifndef SILO_HAS_AA_MUTATION_H
#define SILO_HAS_AA_MUTATION_H

#include <optional>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct HasAAMutation : public Expression {
  private:
   std::string aa_sequence_name;
   unsigned position;

  public:
   explicit HasAAMutation(std::string aa_sequence_name, unsigned position);

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<HasAAMutation>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_HAS_AA_MUTATION_H

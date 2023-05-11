#ifndef SILO_THRESHOLD_H
#define SILO_THRESHOLD_H

#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Threshold : public Operator {
  private:
   std::vector<std::unique_ptr<Operator>> non_negated_children;
   std::vector<std::unique_ptr<Operator>> negated_children;
   unsigned int number_of_matchers;
   bool match_exactly;
   unsigned sequence_count;

  public:
   Threshold(
      std::vector<std::unique_ptr<Operator>>&& non_negated_children,
      std::vector<std::unique_ptr<Operator>>&& negated_children,
      unsigned int number_of_matchers,
      bool match_exactly,
      unsigned sequence_count
   );

   ~Threshold() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString(const Database& database) const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_THRESHOLD_H

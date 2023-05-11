#ifndef SILO_UNION_H
#define SILO_UNION_H
#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::filter_expressions {
class Or;
}  // namespace silo::query_engine::filter_expressions

namespace silo::query_engine::operators {

class Union : public Operator {
   friend class silo::query_engine::filter_expressions::Or;
   std::vector<std::unique_ptr<Operator>> children;

  public:
   explicit Union(std::vector<std::unique_ptr<Operator>>&& children);

   ~Union() noexcept override;

   [[nodiscard]] Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString(const Database& database) const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_UNION_H

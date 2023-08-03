#ifndef SILO_OPERATOR_H
#define SILO_OPERATOR_H

#include <memory>
#include <string>

#include "silo/query_engine/operator_result.h"

namespace silo::query_engine::operators {

enum Type {
   EMPTY,
   FULL,
   INDEX_SCAN,
   INTERSECTION,
   COMPLEMENT,
   RANGE_SELECTION,
   SELECTION,
   BITMAP_SELECTION,
   THRESHOLD,
   UNION,
   BITMAP_PRODUCER
};

class Operator {
  public:
   Operator();

   virtual ~Operator() noexcept;

   [[nodiscard]] virtual Type type() const = 0;

   virtual OperatorResult evaluate() const = 0;

   virtual std::string toString() const = 0;

   virtual std::unique_ptr<Operator> copy() const = 0;

   virtual std::unique_ptr<Operator> negate() const = 0;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_OPERATOR_H

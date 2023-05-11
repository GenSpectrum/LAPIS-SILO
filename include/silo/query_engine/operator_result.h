#ifndef SILO_OPERATOR_RESULT_H
#define SILO_OPERATOR_RESULT_H

#include <roaring/roaring.hh>

namespace silo::query_engine {

/// The return value of the Operator::evaluate method.
/// May return either a mutable or immutable bitmap.
struct OperatorResult {
   roaring::Roaring* mutable_res;
   const roaring::Roaring* immutable_res;

   [[nodiscard]] const roaring::Roaring* getAsConst() const;

   void free() const;
};

}  // namespace silo::query_engine

#endif  // SILO_OPERATOR_RESULT_H

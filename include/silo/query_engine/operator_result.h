#ifndef SILO_OPERATOR_RESULT_H
#define SILO_OPERATOR_RESULT_H

#include <roaring/roaring.hh>

namespace silo::query_engine {

/// The return value of the Operator::evaluate method.
/// May return either a mutable or immutable bitmap.
struct OperatorResult {
  private:
   roaring::Roaring* mutable_bitmap;
   const roaring::Roaring* immutable_bitmap;

  public:
   explicit OperatorResult();
   explicit OperatorResult(const roaring::Roaring* bitmap);
   explicit OperatorResult(roaring::Roaring* bitmap);

   const roaring::Roaring* getConst() const;
   roaring::Roaring* getMutable();
   bool isMutable() const;

   void free() const;
};

}  // namespace silo::query_engine

#endif  // SILO_OPERATOR_RESULT_H

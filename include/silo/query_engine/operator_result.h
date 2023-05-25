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

   // rule of five for manual memory management
   ~OperatorResult();
   OperatorResult(const OperatorResult& other) = delete;
   OperatorResult(OperatorResult&& other) noexcept;
   OperatorResult& operator=(const OperatorResult& other) = delete;
   OperatorResult& operator=(OperatorResult&& other) noexcept;

   std::add_lvalue_reference<roaring::Roaring>::type operator*();
   std::add_lvalue_reference<const roaring::Roaring>::type operator*() const;
   roaring::Roaring* operator->();
   const roaring::Roaring* operator->() const;

   bool isMutable() const;
};

}  // namespace silo::query_engine

#endif  // SILO_OPERATOR_RESULT_H

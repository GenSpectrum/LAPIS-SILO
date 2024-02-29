#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace roaring {
class Roaring;
}  // namespace roaring

namespace silo::query_engine::filter_expressions {
class Expression;
}  // namespace silo::query_engine::filter_expressions

namespace silo::query_engine::operators {

class IndexScan : public Operator {
   friend class Operator;

  private:
   std::optional<std::unique_ptr<query_engine::filter_expressions::Expression>> logical_equivalent;
   const roaring::Roaring* bitmap;
   uint32_t row_count;

  public:
   explicit IndexScan(const roaring::Roaring* bitmap, uint32_t row_count);

   explicit IndexScan(
      std::unique_ptr<query_engine::filter_expressions::Expression>&& logical_equivalent,
      const roaring::Roaring* bitmap,
      uint32_t row_count
   );

   ~IndexScan() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<IndexScan>&& index_scan);
};

}  // namespace silo::query_engine::operators

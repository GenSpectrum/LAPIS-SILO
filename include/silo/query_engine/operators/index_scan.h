#ifndef SILO_INDEX_SCAN_H
#define SILO_INDEX_SCAN_H

#include <cstdint>
#include <memory>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace roaring {
class Roaring;
}  // namespace roaring

namespace silo::query_engine::operators {

class IndexScan : public Operator {
  private:
   const roaring::Roaring* bitmap;
   uint32_t row_count;

  public:
   explicit IndexScan(const roaring::Roaring* bitmap, uint32_t row_count);

   ~IndexScan() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_INDEX_SCAN_H

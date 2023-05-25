#ifndef SILO_INDEX_SCAN_H
#define SILO_INDEX_SCAN_H

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class IndexScan : public Operator {
  private:
   const roaring::Roaring* bitmap;

  public:
   explicit IndexScan(const roaring::Roaring* bitmap);

   ~IndexScan() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_INDEX_SCAN_H

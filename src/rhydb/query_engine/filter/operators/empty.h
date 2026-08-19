#pragma once

#include <memory>
#include <string>

#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/storage/column/row_layout.h"

namespace rhydb::query_engine::filter::operators {

class Empty : public Operator {
  private:
   storage::column::RowLayout row_layout;

  public:
   explicit Empty(storage::column::RowLayout row_layout);

   ~Empty() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Empty>&& empty);
};

}  // namespace rhydb::query_engine::filter::operators

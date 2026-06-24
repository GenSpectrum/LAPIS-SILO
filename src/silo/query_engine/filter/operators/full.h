#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/row_layout.h"

namespace silo::query_engine::filter::operators {

class Full : public Operator {
   storage::column::RowLayout row_layout;

  public:
   explicit Full(storage::column::RowLayout row_layout);

   ~Full() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Full>&& full_operator);
};

}  // namespace silo::query_engine::filter::operators

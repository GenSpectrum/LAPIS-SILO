#pragma once

#include <memory>
#include <string>

#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/storage/column/row_layout.h"

namespace rhydb::query_engine::filter::operators {

class Complement : public Operator {
   friend class Operator;

   std::unique_ptr<Operator> child;
   storage::column::RowLayout row_layout;

  public:
   explicit Complement(std::unique_ptr<Operator> child, storage::column::RowLayout row_layout);

   static std::unique_ptr<Complement> fromDeMorgan(
      OperatorVector disjunction,
      storage::column::RowLayout row_layout
   );

   ~Complement() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Complement>&& complement);
};

}  // namespace rhydb::query_engine::filter::operators

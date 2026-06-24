#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/row_layout.h"

namespace silo::query_engine::filter::operators {

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

}  // namespace silo::query_engine::filter::operators

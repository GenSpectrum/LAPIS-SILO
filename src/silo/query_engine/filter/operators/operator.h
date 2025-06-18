#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"

namespace silo::query_engine::filter::operators {

enum Type {
   EMPTY,
   FULL,
   INDEX_SCAN,
   INTERSECTION,
   COMPLEMENT,
   RANGE_SELECTION,
   SELECTION,
   BITMAP_SELECTION,
   THRESHOLD,
   UNION,
   BITMAP_PRODUCER
};

class Operator {
  public:
   Operator();

   virtual ~Operator() noexcept;

   [[nodiscard]] virtual Type type() const = 0;

   virtual CopyOnWriteBitmap evaluate() const = 0;

   virtual std::string toString() const = 0;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Operator>&& some_operator);
};

using OperatorVector = std::vector<std::unique_ptr<filter::operators::Operator>>;

}  // namespace silo::query_engine::filter::operators

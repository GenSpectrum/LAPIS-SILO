#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

class RangeSelection : public Operator {
  public:
   struct Range {
      uint32_t start;
      uint32_t end;

      explicit Range(uint32_t start, uint32_t end);
   };

  private:
   std::vector<Range> ranges;
   uint32_t row_count;

  public:
   explicit RangeSelection(std::vector<Range>&& ranges, uint32_t row_count);

   ~RangeSelection() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<RangeSelection>&& range_selection);
};

}  // namespace silo::query_engine::filter::operators

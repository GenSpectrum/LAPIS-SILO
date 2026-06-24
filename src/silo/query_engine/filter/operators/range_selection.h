#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/row_layout.h"

namespace silo::query_engine::filter::operators {

class RangeSelection : public Operator {
  public:
   struct Range {
      storage::column::RowId start;
      storage::column::RowId end;

      explicit Range(storage::column::RowId start, storage::column::RowId end);
   };

  private:
   std::vector<Range> ranges;
   storage::column::RowLayout row_layout;

  public:
   explicit RangeSelection(std::vector<Range>&& ranges, storage::column::RowLayout row_layout);

   ~RangeSelection() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<RangeSelection>&& range_selection);
};

}  // namespace silo::query_engine::filter::operators

#ifndef SILO_DATE_COLUMN_H
#define SILO_DATE_COLUMN_H

#include <chrono>

#include <roaring/roaring.hh>

#include "silo/storage/raw_base_column.h"

namespace silo::storage {

class DateColumn : public RawBaseColumn<std::chrono::year_month_day> {
  private:
   const std::string column_name;
   const std::vector<std::chrono::year_month_day> values;

  public:
   using RawBaseColumn<std::chrono::year_month_day>::RawBaseColumn;
};

}  // namespace silo::storage

#endif  // SILO_DATE_COLUMN_H

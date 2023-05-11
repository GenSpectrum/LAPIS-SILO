#ifndef SILO_DATE_COLUMN_H
#define SILO_DATE_COLUMN_H

#include <chrono>

#include <roaring/roaring.hh>

#include "raw_base_column.h"

namespace silo::storage::column {

class DateColumn {
  public:
   virtual roaring::Roaring filterRange(
      const std::chrono::year_month_day& from_date,
      const std::chrono::year_month_day& to_date
   ) const = 0;
};

class RawDateColumn : public DateColumn {
  private:
   const std::string column_name;
   const std::vector<std::chrono::year_month_day> values;

  public:
   RawDateColumn(std::string column_name, std::vector<std::chrono::year_month_day> values);

   virtual roaring::Roaring filterRange(
      const std::chrono::year_month_day& from_date,
      const std::chrono::year_month_day& to_date
   ) const override;
};

class SortedDateColumn : public DateColumn {
  private:
   const std::string column_name;
   const std::vector<std::chrono::year_month_day> values;

  public:
   SortedDateColumn(std::string column_name, std::vector<std::chrono::year_month_day> values);

   virtual roaring::Roaring filterRange(
      const std::chrono::year_month_day& from_date,
      const std::chrono::year_month_day& to_date
   ) const override;
};

}  // namespace silo::storage::column

#endif  // SILO_DATE_COLUMN_H

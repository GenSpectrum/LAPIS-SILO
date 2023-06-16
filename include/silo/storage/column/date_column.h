#ifndef SILO_DATE_COLUMN_H
#define SILO_DATE_COLUMN_H

#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/date.h"
#include "silo/storage/column/column.h"

namespace silo::storage::column {

class DateColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& values;
      archive& is_sorted;
   }

  private:
   std::vector<silo::common::Date> values;
   bool is_sorted;

  public:
   DateColumn();
   explicit DateColumn(bool is_sorted);

   [[nodiscard]] bool isSorted() const;

   void insert(const silo::common::Date& value);

   [[nodiscard]] const std::vector<silo::common::Date>& getValues() const;

   std::string getAsString(std::size_t idx) const override;
};

}  // namespace silo::storage::column

#endif  // SILO_DATE_COLUMN_H

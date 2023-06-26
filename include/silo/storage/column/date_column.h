#ifndef SILO_DATE_COLUMN_H
#define SILO_DATE_COLUMN_H

#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/date.h"
#include "silo/storage/column/column.h"

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class DateColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& values;
      archive& is_sorted;
      // clang-format on
   }

   std::vector<silo::common::Date> values;
   bool is_sorted;

  public:
   explicit DateColumnPartition(bool is_sorted);

   [[nodiscard]] bool isSorted() const;

   void insert(const silo::common::Date& value);

   [[nodiscard]] const std::vector<silo::common::Date>& getValues() const;
};

class DateColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& is_sorted;
      // clang-format on
   }

  private:
   bool is_sorted;
   DateColumn();

  public:
   explicit DateColumn(bool is_sorted);

   DateColumnPartition createPartition();
};

}  // namespace silo::storage::column

#endif  // SILO_DATE_COLUMN_H

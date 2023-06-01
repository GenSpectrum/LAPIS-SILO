#ifndef SILO_DATE_COLUMN_H
#define SILO_DATE_COLUMN_H

#include <ctime>
#include <vector>

#include <roaring/roaring.hh>

namespace silo::storage::column {

class DateColumn {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& values;
      archive& is_sorted;
   }

  private:
   std::vector<std::time_t> values;
   bool is_sorted;

  public:
   DateColumn();
   DateColumn(bool is_sorted);

   bool isSorted() const;

   void insert(const std::time_t& value);

   const std::vector<std::time_t>& getValues() const;
};

}  // namespace silo::storage::column

#endif  // SILO_DATE_COLUMN_H

#ifndef SILO_INT_COLUMN_H
#define SILO_INT_COLUMN_H

#include <string>
#include <vector>

#include "silo/storage/column/column.h"

namespace silo::storage::column {

class IntColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& values;
   }

  private:
   std::vector<uint64_t> values;

  public:
   IntColumn();

   [[nodiscard]] const std::vector<uint64_t>& getValues() const;

   void insert(uint64_t value);

   [[nodiscard]] std::string getAsString(std::size_t idx) const override;
};

}  // namespace silo::storage::column

#endif  // SILO_INT_COLUMN_H

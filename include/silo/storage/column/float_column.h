#ifndef SILO_FLOAT_COLUMN_H
#define SILO_FLOAT_COLUMN_H

#include <string>
#include <vector>

#include "silo/storage/column/column.h"

namespace silo::storage::column {

class FloatColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& values;
   }

  private:
   std::vector<double> values;

  public:
   FloatColumn();

   [[nodiscard]] const std::vector<double>& getValues() const;

   void insert(double value);

   [[nodiscard]] std::string getAsString(std::size_t idx) const override;
};

}  // namespace silo::storage::column

#endif  // SILO_FLOAT_COLUMN_H

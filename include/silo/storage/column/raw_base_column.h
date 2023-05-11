#ifndef SILO_METADATA_COLUMN_H
#define SILO_METADATA_COLUMN_H

#include <chrono>
#include <string>
#include <vector>

#include <roaring/roaring.hh>

namespace silo::storage::column {

template <typename T>
class RawBaseColumn {
  private:
   std::string column_name;
   std::vector<T> values;

  public:
   RawBaseColumn(std::string column_name, std::vector<T> values);

   roaring::Roaring filter(const T& value) const;
};

}  // namespace silo::storage::column

#endif  // SILO_METADATA_COLUMN_H

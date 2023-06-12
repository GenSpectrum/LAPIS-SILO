#ifndef SILO_COLUMN_H
#define SILO_COLUMN_H

#include <cstddef>
#include <cstdint>

namespace silo::storage::column {

class Column {
  public:
   virtual std::string getAsString(std::size_t idx) const = 0;
};

}  // namespace silo::storage::column

#endif  // SILO_COLUMN_H

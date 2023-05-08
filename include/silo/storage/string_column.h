#ifndef SILO_STRING_COLUMN_H
#define SILO_STRING_COLUMN_H

#include <string>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/storage/raw_base_column.h"

namespace silo {

class Dictionary;

namespace storage {

class StringColumn {
  public:
   virtual roaring::Roaring filter(std::string value) const = 0;
};

class RawStringColumn : public RawBaseColumn<std::string>, public StringColumn {
  private:
   std::string column_name;
   std::vector<std::string> values;

  public:
   using RawBaseColumn<std::string>::RawBaseColumn;

   virtual roaring::Roaring filter(std::string value) const override;
};

class IndexedStringColumn : public StringColumn {
  private:
   std::string column_name;
   const silo::Dictionary& dictionary;
   std::vector<roaring::Roaring> indexed_values;

  public:
   IndexedStringColumn(
      std::string column_name,
      const silo::Dictionary& dictionary,
      std::vector<roaring::Roaring> indexed_values
   );

   virtual roaring::Roaring filter(std::string value) const override;
};

}  // namespace storage
}  // namespace silo

#endif  // SILO_STRING_COLUMN_H

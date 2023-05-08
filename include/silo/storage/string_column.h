#ifndef SILO_STRING_COLUMN_H
#define SILO_STRING_COLUMN_H

#include <string>
#include <vector>

#include <roaring/roaring.hh>

namespace silo {

class Dictionary;

namespace storage {

class StringColumn {
  public:
   virtual const roaring::Roaring filter(std::string value) const = 0;
};

class RawStringColumn : public StringColumn {
  private:
   std::string columnName;
   std::vector<std::string> values;

  public:
   RawStringColumn(std::string columnName, std::vector<std::string> values);

   virtual const roaring::Roaring filter(std::string value) const override;
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

   virtual const roaring::Roaring filter(std::string value) const override;
};

}  // namespace storage
}  // namespace silo

#endif  // SILO_STRING_COLUMN_H

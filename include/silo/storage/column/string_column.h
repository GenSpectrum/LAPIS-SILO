#ifndef SILO_STRING_COLUMN_H
#define SILO_STRING_COLUMN_H

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

#include "silo/storage/column/column.h"

namespace silo::storage::column {

class RawStringColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& values;
   }

  private:
   std::vector<std::string> values;

  public:
   RawStringColumn();

   [[nodiscard]] const std::vector<std::string>& getValues() const;

   void insert(const std::string& value);

   [[nodiscard]] std::string getAsString(std::size_t idx) const override;
};

class IndexedStringColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& value_ids;
      archive& value_to_id_lookup;
      archive& id_to_value_lookup;
      archive& indexed_values;
   }

  private:
   std::vector<uint32_t> value_ids;
   std::unordered_map<std::string, uint32_t> value_to_id_lookup;
   std::vector<std::string> id_to_value_lookup;
   std::vector<roaring::Roaring> indexed_values;

  public:
   IndexedStringColumn();

   [[nodiscard]] roaring::Roaring filter(const std::string& value) const;

   void insert(const std::string& value);

   [[nodiscard]] std::string getAsString(std::size_t idx) const override;
};

}  // namespace silo::storage::column

#endif  // SILO_STRING_COLUMN_H

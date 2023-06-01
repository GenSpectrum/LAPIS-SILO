#ifndef SILO_STRING_COLUMN_H
#define SILO_STRING_COLUMN_H

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

namespace silo::storage::column {

class RawStringColumn {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& values;
   }

  private:
   std::vector<std::string> values;

  public:
   RawStringColumn();

   const std::vector<std::string>& getValues() const;

   void insert(const std::string& value);
};

class IndexedStringColumn {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& value_id_lookup;
      archive& indexed_values;
      archive& sequence_count;
   }

  private:
   std::unordered_map<std::string, uint32_t> value_id_lookup;
   std::vector<roaring::Roaring> indexed_values;
   uint32_t sequence_count = 0;

  public:
   IndexedStringColumn();

   roaring::Roaring filter(const std::string& value) const;

   void insert(const std::string& value);
};

}  // namespace silo::storage::column

#endif  // SILO_STRING_COLUMN_H

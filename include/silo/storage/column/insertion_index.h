#ifndef SILO_INSERTION_INDEX_H
#define SILO_INSERTION_INDEX_H

#include <array>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

#include "silo/common/nucleotide_symbol_map.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/template_utils.h"

namespace boost::serialization {
struct access;
}  // namespace boost::serialization

namespace silo::storage::column::insertion {

using ThreeMer = std::array<NUCLEOTIDE_SYMBOL, 3>;
using InsertionIds = std::vector<uint32_t>;

class Insertion {
  private:
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
         archive & value;
         archive & sequence_ids;
      // clang-format on
   }

  public:
   std::string value;
   roaring::Roaring sequence_ids;
};

template <typename Symbol>
class InsertionPosition {
  private:
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
         archive & insertions;
         archive & three_mer_index;
      // clang-format on
   }

   static constexpr size_t THREE_DIMENSIONS = 3;

  public:
   using ThreeMerIndex =
      NestedContainer<THREE_DIMENSIONS, silo::NucleotideSymbolMap, InsertionIds>::type;

   std::vector<Insertion> insertions;
   ThreeMerIndex three_mer_index;

   std::unique_ptr<roaring::Roaring> searchWithThreeMerIndex(
      const std::vector<ThreeMer>& search_three_mers,
      const std::regex& search_pattern
   ) const;

   std::unique_ptr<roaring::Roaring> searchWithRegex(const std::regex& regex_search_pattern) const;

   void buildThreeMerIndex();

   std::unique_ptr<roaring::Roaring> search(const std::string& search_pattern) const;
};

template <typename Symbol>
class InsertionIndex {
  private:
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & insertion_positions;
      archive & collected_insertions;
      // clang-format on
   }

   std::unordered_map<uint32_t, InsertionPosition<Symbol>> insertion_positions;
   std::unordered_map<uint32_t, std::unordered_map<std::string, roaring::Roaring>>
      collected_insertions;

  public:
   void addLazily(const std::string& insertions_string, uint32_t sequence_id);

   void buildIndex();

   std::unique_ptr<roaring::Roaring> search(uint32_t position, const std::string& search_pattern)
      const;
};

}  // namespace silo::storage::column::insertion

#endif  // SILO_INSERTION_INDEX_H

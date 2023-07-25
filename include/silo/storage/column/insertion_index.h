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

namespace boost::serialization {
struct access;
}  // namespace boost::serialization

namespace silo::storage::column::insertion {

class InsertionIndex {
   friend class boost::serialization::access;

  public:
   using three_mer_t = std::array<NUCLEOTIDE_SYMBOL, 3>;
   using sequence_ids_t = roaring::Roaring;
   using insertion_ids_t = std::vector<uint32_t>;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & insertion_positions;
      archive & collected_insertions;
      // clang-format on
   }

   struct Insertion {
      friend class boost::serialization::access;

      template <class Archive>
      [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
         // clang-format off
         archive & value;
         archive & sequence_ids;
         // clang-format on
      }

      std::string value;
      sequence_ids_t sequence_ids;
   };

   using one_mer_index_t = silo::NucleotideSymbolMap<insertion_ids_t>;
   using two_mer_index_t = silo::NucleotideSymbolMap<one_mer_index_t>;
   using three_mer_index_t = silo::NucleotideSymbolMap<two_mer_index_t>;

   struct InsertionPosition {
      friend class boost::serialization::access;

      template <class Archive>
      [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
         // clang-format off
         archive & insertions;
         archive & three_mer_index;
         // clang-format on
      }

      std::vector<Insertion> insertions;
      three_mer_index_t three_mer_index;

      void buildThreeMerIndex();

      std::unique_ptr<sequence_ids_t> searchWithThreeMerIndex(
         const std::vector<three_mer_t>& search_three_mers,
         const std::regex& search_pattern
      ) const;
   };

   std::unordered_map<uint32_t, InsertionPosition> insertion_positions;
   std::unordered_map<uint32_t, std::unordered_map<std::string, sequence_ids_t>>
      collected_insertions;

  public:
   void addLazily(const std::string& insertions_string, uint32_t sequence_id);

   void buildIndex();

   std::unique_ptr<roaring::Roaring> search(uint32_t position, const std::string& search_pattern)
      const;
};

}  // namespace silo::storage::column::insertion

#endif  // SILO_INSERTION_INDEX_H

#pragma once

#include <cstddef>
#include <map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

namespace silo::storage::column {

class HorizontalCoverageIndex {
  public:
   const size_t genome_length;

   std::map<uint32_t, roaring::Roaring> horizontal_bitmaps;
   std::vector<std::pair<uint32_t, uint32_t>> start_end;

   explicit HorizontalCoverageIndex(uint32_t genome_length)
       : genome_length(genome_length) {}

   void insertCoverage(
      uint32_t start,
      uint32_t end,
      const std::vector<uint32_t>& positions_with_symbol_missing
   );

   void insertNullSequence();

   template <typename SymbolType>
   void insertSequenceCoverage(std::string sequence, uint32_t offset);

   template <typename SymbolType>
   void overwriteCoverageInSequence(
      std::vector<std::string>& sequences,
      const roaring::Roaring& row_ids
   ) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      archive & horizontal_bitmaps;
      archive & start_end;
   }
};

}  // namespace silo::storage::column

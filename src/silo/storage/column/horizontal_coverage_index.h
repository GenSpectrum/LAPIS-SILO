#pragma once

#include <map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

namespace silo::storage::column {

template <typename SymbolType>
class HorizontalCoverageIndex {
  public:
   const size_t genome_length;

   std::map<size_t, roaring::Roaring> horizontal_bitmaps;
   std::vector<std::pair<size_t, size_t>> start_end;

   explicit HorizontalCoverageIndex(size_t genome_length)
       : genome_length(genome_length) {}

   void insertNullSequence();

   void insertCoverage(size_t sequence_idx, std::string sequence, uint32_t offset);

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

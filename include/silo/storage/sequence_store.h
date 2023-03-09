
#ifndef SILO_SEQUENCE_STORE_H
#define SILO_SEQUENCE_STORE_H

#include "meta_store.h"
#include "silo/roaring/roaring.hh"
#include "silo/roaring/roaring_serialize.h"

namespace silo {

struct Position {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& flipped_bitmap;
      ar& bitmaps;
      ar& N_indexed;
   }

   roaring::Roaring bitmaps[SYMBOL_COUNT];
   // Reference bitmap is flipped
   uint32_t flipped_bitmap = UINT32_MAX;
   bool N_indexed = false;
};

class SequenceStore {
  private:
   unsigned sequence_count;

  public:
   friend class CompressedSequenceStore;
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& sequence_count;
      ar& positions;
      ar& N_bitmaps;
   }
   Position positions[GENOME_LENGTH];
   std::vector<roaring::Roaring> N_bitmaps;

   [[nodiscard]] size_t computeSize() const {
      size_t result = 0;
      for (auto& p : positions) {
         for (auto& b : p.bitmaps) {
            result += b.getSizeInBytes(false);
         }
      }
      return result;
   }

   /// default constructor
   SequenceStore() {}

   /// pos: 1 indexed position of the genome
   [[nodiscard]] const roaring::Roaring* bm(size_t pos, GENOME_SYMBOL s) const {
      return &positions[pos - 1].bitmaps[s];
   }

   /// Returns an Roaring-bitmap which has the given residue r at the position pos,
   /// where the residue is interpreted in the _a_pproximate meaning
   /// That means a symbol matches all mixed symbols, which can indicate the residue
   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring* bma(size_t pos, GENOME_SYMBOL r) const;

   /// Same as before for flipped bitmaps for r
   [[nodiscard]] roaring::Roaring* bma_neg(size_t pos, GENOME_SYMBOL r) const;

   void interpret(const std::vector<std::string>& genomes);

   void indexAllN();

   void indexAllN_naive();

   int db_info(std::ostream& io) const;
};

[[maybe_unused]] unsigned save_db(const SequenceStore& db, const std::string& db_filename);

[[maybe_unused]] unsigned load_db(SequenceStore& db, const std::string& db_filename);

[[maybe_unused]] unsigned runOptimize(SequenceStore& db);

[[maybe_unused]] unsigned shrinkToFit(SequenceStore& db);

}  // namespace silo

#endif  // SILO_SEQUENCE_STORE_H

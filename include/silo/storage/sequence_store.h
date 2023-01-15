//
// Created by Alexander Taepper on 26.09.22.
//

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
   }

   roaring::Roaring bitmaps[symbolCount];
   // Reference bitmap is flipped
   uint32_t flipped_bitmap = UINT32_MAX;
   bool N_indexed = false;
};

class SequenceStore;

class CompressedSequenceStore {
   public:
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& sequence_count;
      ar& positions;
      ar& start_gaps;
      ar& end_gaps;
   }

   explicit CompressedSequenceStore() : sequence_count(0) {
      start_gaps = std::vector<uint32_t>(0);
      end_gaps = std::vector<uint32_t>(0);
   }

   explicit CompressedSequenceStore(const SequenceStore& seq_store);

   std::pair<size_t, size_t> size() const {
      size_t size_portable = 0;
      size_t size = 0;
      for (auto& position : positions) {
         for (auto& bm : position.bitmaps) {
            size_portable += bm.getSizeInBytes(true);
            size += bm.getSizeInBytes(false);
         }
      }
      return {size_portable, size};
   }

   Position positions[genomeLength];
   std::vector<uint32_t> start_gaps;
   std::vector<uint32_t> end_gaps;
   unsigned sequence_count;
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
   }
   Position positions[genomeLength];
   std::vector<roaring::Roaring> N_bitmaps;

   [[nodiscard]] std::pair<size_t, size_t> computeSize() const {
      size_t result_port = 0;
      size_t result = 0;
      for (auto& p : positions) {
         for (auto& b : p.bitmaps) {
            result_port += b.getSizeInBytes(true);
            result += b.getSizeInBytes(false);
         }
      }
      return {result_port, result};
   }

   /// default constructor
   SequenceStore() {}

   /// decompress sequence_store
   explicit SequenceStore(const CompressedSequenceStore& c_seq_store);

   /// pos: 1 indexed position of the genome
   [[nodiscard]] const roaring::Roaring* bm(size_t pos, Symbol s) const {
      return &positions[pos - 1].bitmaps[s];
   }

   /// Returns an Roaring-bitmap which has the given residue r at the position pos,
   /// where the residue is interpreted in the _a_pproximate meaning
   /// That means a symbol matches all mixed symbols, which can indicate the residue
   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring* bma(size_t pos, Symbol r) const;

   /// Same as before for flipped bitmaps for r
   [[nodiscard]] roaring::Roaring* bma_neg(size_t pos, Symbol r) const;

   void interpret(const std::vector<std::string>& genomes);

   void indexAllN();

   void indexAllN_naive();

   int db_info(std::ostream& io) const;
};

[[maybe_unused]] unsigned save_db(const SequenceStore& db, const std::string& db_filename);

[[maybe_unused]] unsigned load_db(SequenceStore& db, const std::string& db_filename);

[[maybe_unused]] unsigned runOptimize(SequenceStore& db);

[[maybe_unused]] unsigned shrinkToFit(SequenceStore& db);

} //namespace silo;

#endif //SILO_SEQUENCE_STORE_H

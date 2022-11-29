//
// Created by Alexander Taepper on 26.09.22.
//

#ifndef SILO_SEQUENCE_STORE_H
#define SILO_SEQUENCE_STORE_H

#include "meta_store.h"

namespace silo {

struct Position {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& reference;
      ar& bitmaps;
   }

   roaring::Roaring bitmaps[symbolCount];
   unsigned int reference;
};

class SequenceStore {
   private:
   unsigned sequence_count;

   public:
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& sequence_count;
      ar& positions;
   }
   Position positions[genomeLength];

   [[nodiscard]] size_t computeSize() const {
      size_t result = 0;
      for (auto& p : positions) {
         for (auto& b : p.bitmaps) {
            result += b.getSizeInBytes();
         }
      }
      return result;
   }

   /// pos: 1 indexed position of the genome
   [[nodiscard]] const roaring::Roaring* bm(size_t pos, Symbol s) const {
      return &positions[pos - 1].bitmaps[s];
   }

   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring bmr(size_t pos, std::string s) const {
      return positions[pos - 1].bitmaps[to_symbol(s.at(pos - 1))];
   }

   /// Returns an Roaring-bitmap which has the given residue r at the position pos,
   /// where the residue is interpreted in the _a_pproximate meaning
   /// That means a symbol matches all mixed symbols, which can indicate the residue
   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring bma(size_t pos, Residue r) const;

   void interpret(const std::vector<std::string>& genomes);

   void interpret_offset_p(const std::vector<std::string>& genomes, uint32_t offset);

   int db_info(std::ostream& io) const;
};

[[maybe_unused]] unsigned save_db(const SequenceStore& db, const std::string& db_filename);

[[maybe_unused]] unsigned load_db(SequenceStore& db, const std::string& db_filename);

[[maybe_unused]] unsigned runoptimize(SequenceStore& db);

} //namespace silo;

#endif //SILO_SEQUENCE_STORE_H

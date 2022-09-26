//
// Created by Alexander Taepper on 26.09.22.
//


#include "meta_store.h"

struct Position {
   friend class boost::serialization::access;
   template<class Archive>
   void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
   {
      unsigned i;
      for(i = 0; i < symbolCount; ++i)
         ar & bitmaps[i];
   }
   roaring::Roaring bitmaps[symbolCount];
};

struct SequenceStore {
   friend class boost::serialization::access;
   template<class Archive>
   void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
   {
      ar & sequenceCount;
      unsigned i;
      for(i = 0; i < genomeLength; ++i)
         ar & positions[i];
   }

   Position positions[genomeLength];
   unsigned sequenceCount = 0;

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
   [[nodiscard]] const roaring::Roaring *bm(size_t pos, Symbol s) const {
      return &positions[pos-1].bitmaps[s];
   }

   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring bmr(size_t pos, string s) const {
      return positions[pos-1].bitmaps[to_symbol(s.at(pos-1))];
   }

   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring ref_mut(size_t pos, string s) const {
      Roaring tmp =  bmr(pos, std::move(s));
      tmp.flip(0, sequenceCount);
      return tmp;
   }

   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring neg_bm(size_t pos, Symbol s) const {
      Roaring tmp = *bm(pos, s);
      tmp.flip(0, sequenceCount);
      return tmp;
   }

   /// Returns an Roaring-bitmap which has the given residue r at the position pos,
   /// where the residue is interpreted in the _a_pproximate meaning
   /// That means a symbol matches all mixed symbols, which can indicate the residue
   /// pos: 1 indexed position of the genome
   [[nodiscard]] roaring::Roaring bma(size_t pos, Residue r) const;
};
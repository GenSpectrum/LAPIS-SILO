//
// Created by Alexander Taepper on 26.09.22.
//

#ifndef SILO_SEQUENCE_STORE_H
#define SILO_SEQUENCE_STORE_H

#include "meta_store.h"

namespace silo {

   struct Position {
      friend class boost::serialization::access;

      template<class Archive>
      void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
         unsigned i;
         for (i = 0; i < symbolCount; ++i)
            ar & bitmaps[i];
      }

      roaring::Roaring bitmaps[symbolCount];
   };

   struct SequenceStore {
      friend class boost::serialization::access;

      template<class Archive>
      void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
         ar & sequenceCount;
         unsigned i;
         for (i = 0; i < genomeLength; ++i)
            ar & positions[i];
      }

      Position positions[genomeLength];
      unsigned sequenceCount = 0;

      [[nodiscard]] size_t computeSize() const {
         size_t result = 0;
         for (auto &p: positions) {
            for (auto &b: p.bitmaps) {
               result += b.getSizeInBytes();
            }
         }
         return result;
      }

      /// pos: 1 indexed position of the genome
      [[nodiscard]] const roaring::Roaring *bm(size_t pos, Symbol s) const {
         return &positions[pos - 1].bitmaps[s];
      }

      /// pos: 1 indexed position of the genome
      [[nodiscard]] roaring::Roaring bmr(size_t pos, string s) const {
         return positions[pos - 1].bitmaps[to_symbol(s.at(pos - 1))];
      }

      /// pos: 1 indexed position of the genome
      [[nodiscard]] roaring::Roaring ref_mut(size_t pos, string s) const {
         Roaring tmp = bmr(pos, std::move(s));
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

   int db_info(const SequenceStore &db, ostream &io);

   unsigned save_db(const SequenceStore &db, const std::string &db_filename);

   unsigned load_db(SequenceStore &db, const std::string &db_filename);

   // static void interpret(SequenceStore& db, const vector<string>& genomes);

   void process(SequenceStore &db, istream &in);

   // static void interpret_offset(SequenceStore& db, const vector<string>& genomes, uint32_t offset);

   void process_partitioned_on_the_fly(SequenceStore &db, MetaStore &mdb, istream &in);

   // static void interpret_specific(SequenceStore& db, const vector<pair<uint64_t, string>>& genomes);

   void partition(MetaStore &mdb, istream &in, const string &output_prefix_);

} //namespace silo;

#endif //SILO_SEQUENCE_STORE_H


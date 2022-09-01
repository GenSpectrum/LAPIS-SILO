#include "silo.h"

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
   [[nodiscard]] const roaring::Roaring * bm(size_t pos, Symbol s) const {
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

roaring::Roaring SequenceStore::bma(size_t pos, Residue r) const {
   switch(r){
      case aA:{
         const roaring::Roaring* tmp[8] = {bm(pos, A), bm(pos, N),
                                           bm(pos, R), bm(pos, W), bm(pos, M),
                                           bm(pos, D), bm(pos, H), bm(pos, V)};
         return roaring::Roaring::fastunion(8, tmp);
      }
      case aC:{
         const roaring::Roaring* tmp[8] = {bm(pos, C), bm(pos, N),
                                           bm(pos, Y), bm(pos, S), bm(pos, M),
                                           bm(pos, B), bm(pos, H), bm(pos, V)};
         return roaring::Roaring::fastunion(8, tmp);
      }
      case aG:{
         const roaring::Roaring* tmp[8] = {bm(pos, G), bm(pos, N),
                                           bm(pos, R), bm(pos, S), bm(pos, K),
                                           bm(pos, D), bm(pos, B), bm(pos, V)};
         return roaring::Roaring::fastunion(8, tmp);
      }
      case aT:{
         const roaring::Roaring* tmp[8] = {bm(pos, T), bm(pos, N),
                                           bm(pos, Y), bm(pos, W), bm(pos, K),
                                           bm(pos, D), bm(pos, H), bm(pos, B)};
         return roaring::Roaring::fastunion(8, tmp);
      }
   }
   cerr << "Should not happen, number of residue changed?" << endl;
   return roaring::Roaring{};
}


int db_info(const unique_ptr<SequenceStore>& db, ostream& io){
   io << "sequence count: " << db->sequenceCount << endl;
   io << "total size: " << db->computeSize() << endl;
   return 0;
}

static unsigned save_db(const SequenceStore& db, const std::string& db_filename) {
   std::cout << "Writing out db." << std::endl;

   ofstream wf(db_filename, ios::out | ios::binary);
   if(!wf) {
      cerr << "Cannot open ofile: " << db_filename << endl;
      return 1;
   }

   {
      boost::archive::binary_oarchive oa(wf);
      // write class instance to archive
      oa << db;
      // archive and stream closed when destructors are called
   }

   return 0;
}

static unsigned load_db(SequenceStore* db, const std::string& db_filename) {
   {
      // create and open an archive for input
      std::ifstream ifs(db_filename, ios::binary);
      boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> *db;
      // archive and stream closed when destructors are called
   }
   return 0;
}


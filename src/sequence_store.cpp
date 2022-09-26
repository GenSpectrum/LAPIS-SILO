//
// Created by Alexander Taepper on 01.09.22.
//

#include "meta_store.cpp"

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


int db_info(const SequenceStore& db, ostream& io){
   io << "sequence count: " << db.sequenceCount << endl;
   io << "total size: " << db.computeSize() << endl;
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

static unsigned load_db(SequenceStore& db, const std::string& db_filename) {
   {
      // create and open an archive for input
      std::ifstream ifs(db_filename, ios::binary);
      boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> db;
      // archive and stream closed when destructors are called
   }
   return 0;
}

static void interpret(SequenceStore& db, const vector<string>& genomes);

static void process(SequenceStore& db, istream& in) {
   static constexpr unsigned chunkSize = 1024;

   vector<string> genomes;
   while (true) {
      string name, genome;
      if (!getline(in, name) || name.empty()) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return;
      }
      genomes.push_back(std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret(db, genomes);
         genomes.clear();
      }
   }
   interpret(db, genomes);
   cout << "sequence count: " << db.sequenceCount << endl;
   cout << "total size: " << db.computeSize() << endl;
   // TODO think about return type
}


static void interpret_offset(SequenceStore& db, const vector<string>& genomes, uint32_t offset){
   vector<unsigned> offsets[symbolCount];
   for (unsigned index = 0; index != genomeLength; ++index) {
      for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
         char c = genomes[index2][index];
         Symbol s = to_symbol(c);
         offsets[s].push_back(offset + index2);
      }
      for (unsigned index2 = 0; index2 != symbolCount; ++index2)
         if (!offsets[index2].empty()) {
            db.positions[index].bitmaps[index2].addMany(offsets[index2].size(), offsets[index2].data());
            offsets[index2].clear();
         }
   }
   db.sequenceCount += genomes.size();
}

static void interpret(SequenceStore& db, const vector<string>& genomes){
   // Putting sequences to the end is the same as offsetting them to sequence_count
   interpret_offset(db, genomes, db.sequenceCount);
}


static void process_partitioned_on_the_fly(SequenceStore& db, MetaStore& mdb, istream& in) {
   static constexpr unsigned chunkSize = 1024;

   vector<uint32_t> dynamic_offsets(mdb.pid_to_offset);

   vector<vector<string>> pid_to_genomes;
   pid_to_genomes.resize(mdb.pid_count + 1);
   while (true) {
      string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      uint16_t pid;
      if(mdb.epi_to_pid.contains(epi)) {
         pid = mdb.epi_to_pid.at(epi);
      }
      else{
         pid = mdb.pid_count;
      }
      auto& genomes = pid_to_genomes[pid];
      genomes.emplace_back(std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret_offset(db, genomes, dynamic_offsets[pid]);
         dynamic_offsets[pid] += genomes.size();
         genomes.clear();
      }
   }
   for(uint16_t pid = 0; pid < mdb.pid_count+1; pid++){
      interpret_offset(db, pid_to_genomes[pid], dynamic_offsets[pid]);
   }
   cout << "sequence count: " << db.sequenceCount << endl;
   cout << "total size: " << db.computeSize() << endl;
}

static void interpret_ordered(SequenceStore& db, const vector<pair<uint64_t, string>>& genomes){
   vector<unsigned> offsets[symbolCount];
   for (unsigned index = 0; index != genomeLength; ++index) {
      for (const auto & idx_genome : genomes) {
         char c = idx_genome.second[index];
         Symbol s = to_symbol(c);
         offsets[s].push_back(idx_genome.first);
      }
      for (unsigned index2 = 0; index2 != symbolCount; ++index2) {
         if (!offsets[index2].empty()) {
            db.positions[index].bitmaps[index2].addMany(offsets[index2].size(), offsets[index2].data());
            offsets[index2].clear();
         }
      }
   }
   db.sequenceCount += genomes.size();
}

static void partition(MetaStore &mdb, istream& in, const string& output_prefix_){
   cout << "Now partitioning fasta file to " << output_prefix_ << endl;
   vector<unique_ptr<ostream>> pid_to_ostream;
   const string output_prefix = output_prefix_ + '_';
   for(auto& x : mdb.pid_to_pango){
      auto out = make_unique<ofstream>(output_prefix + x + ".fasta");
      pid_to_ostream.emplace_back(std::move(out));
   }
   cout << "Created file streams for  " << output_prefix_ << endl;
   ofstream undefined_pid_ostream(output_prefix + "NOMETADATA.fasta.xz");
   while (true) {
      string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      if(mdb.epi_to_pid.contains(epi)) {
         auto pid = mdb.epi_to_pid.at(epi);
         *pid_to_ostream[pid] << epi_isl << endl << genome << endl;
      }
      else{
         undefined_pid_ostream << epi_isl << endl << genome << endl;
      }
   }
   cout << "Finished partitioning to " << output_prefix_ << endl;
}


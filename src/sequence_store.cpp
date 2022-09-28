//
// Created by Alexander Taepper on 01.09.22.
//
#include "sequence_store.h"

using namespace silo;

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


int silo::db_info(const SequenceStore& db, ostream& io){
   io << "sequence count: " << number_fmt(db.sequenceCount) << endl;
   io << "total size: " << number_fmt(db.computeSize()) << endl;
   return 0;
}

int silo::db_info_detailed(const SequenceStore& db, ostream& io){
   db_info(db, io);
   vector<size_t> size_by_symbols;
   size_by_symbols.resize(symbolCount);
   for(const auto & position : db.positions){
      for(unsigned symbol = 0; symbol < symbolCount; symbol++){
         size_by_symbols[symbol] += position.bitmaps[symbol].getSizeInBytes();
      }
   }
   for(unsigned symbol = 0; symbol < symbolCount; symbol++){
      io << "size for symbol '" << symbol_rep[symbol] << "': "
         << number_fmt(size_by_symbols[symbol]) << endl;
   }
   return 0;
}

unsigned silo::save_db(const SequenceStore& db, const std::string& db_filename) {
   std::cout << "Writing out db." << std::endl;

   ofstream wf(db_filename, ios::out | ios::binary);
   if(!wf) {
      cerr << "Cannot open ofile: " << db_filename << endl;
      return 1;
   }

   {
      ::boost::archive::binary_oarchive oa(wf);
      // write class instance to archive
      oa << db;
      // archive and stream closed when destructors are called
   }

   return 0;
}

unsigned silo::load_db(SequenceStore& db, const std::string& db_filename) {
   {
      // create and open an archive for input
      std::ifstream ifs(db_filename, ios::binary);
      ::boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> db;
      // archive and stream closed when destructors are called
   }
   return 0;
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

void silo::process_raw(SequenceStore& db, istream& in) {
   static constexpr unsigned chunkSize = 1024;

   vector<string> genomes;
   while (true) {
      string epi_isl, genome;
      if (!getline(in, epi_isl) || epi_isl.empty()) break;
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
   db_info(db,cout);
}


void silo::process(SequenceStore& db, MetaStore& mdb, istream& in) {
   static constexpr unsigned chunkSize = 1024;

   uint32_t sid_ctr = db.sequenceCount;
   vector<string> genomes;
   while (true) {
      string epi_isl, genome;
      if (!getline(in, epi_isl) || epi_isl.empty()) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      genomes.push_back(std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret(db, genomes);
         genomes.clear();
      }

      uint32_t sid = sid_ctr++;
      db.epi_to_sid[epi] = sid;
      db.sid_to_epi.push_back(epi);
   }
   interpret(db, genomes);
   db_info(db,cout);
}

void silo::calc_partition_offsets(SequenceStore& db, MetaStore& mdb, istream& in){
   cout << "Now calculating partition offsets" << endl;

   // Clear the vector and resize
   // TODO for future proofing, instead of clearing, extending them?
   db.pid_to_offset.clear();
   db.pid_to_offset.resize(mdb.pid_count);
   db.pid_to_realcount.clear();
   db.pid_to_realcount.resize(mdb.pid_count);

   while (true) {
      string epi_isl;
      if (!getline(in, epi_isl)) break;
      in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // Add the count to the respective pid
      uint64_t epi = stoi(epi_isl.substr(9));

      if(!mdb.epi_to_pid.contains(epi)) {
         // TODO logging
         continue;
      }

      uint16_t pid = mdb.epi_to_pid[epi];
      db.pid_to_realcount[pid]++;
   }

   // Escalate offsets from start to finish
   uint32_t cumulative_offset = 0;
   for(int i = 0; i<mdb.pid_count; i++){
      db.pid_to_offset[i] += cumulative_offset;
      cumulative_offset += db.pid_to_realcount[i];
   }

   // cumulative_offset should be equal to sequence count now

   cout << "Finished calculating partition offsets." << endl;
}


// TODO this clears the SequenceStore? noo doesn't have to be...
void silo::process_partitioned_on_the_fly(SequenceStore& db, MetaStore& mdb, istream& in) {
   static constexpr unsigned chunkSize = 1024;

   // these offsets lag by chunk.
   vector<uint32_t> dynamic_offsets(db.pid_to_offset);
   // actually these are the same offsets just without lagging by chunk.
   vector<uint32_t> sid_ctrs(db.pid_to_offset);
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

      if(!mdb.epi_to_pid.contains(epi)) {
         // TODO logging
         continue;
      }

      uint16_t pid = mdb.epi_to_pid.at(epi);
      db.pid_to_realcount[pid]++;

      auto& genomes = pid_to_genomes[pid];
      genomes.emplace_back(std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret_offset(db, genomes, dynamic_offsets[pid]);
         dynamic_offsets[pid] += genomes.size();
         genomes.clear();
      }

      uint32_t sid = sid_ctrs[pid]++;
      db.epi_to_sid[epi] = sid;
   }

   for(uint16_t pid = 0; pid < mdb.pid_count+1; pid++){
      interpret_offset(db, pid_to_genomes[pid], dynamic_offsets[pid]);
   }

   // now also calculate the reverse direction for the epi<->sid relationship.
   db.sid_to_epi.resize(db.epi_to_sid.size());
   for(auto& x : db.epi_to_sid){
      db.sid_to_epi[x.second] = x.first;
   }

   db_info(db, cout);
}

// Only for testing purposes. Very inefficient. Will insert the genome in specific positions to the sequenceStore
void interpret_specific(SequenceStore& db, const vector<pair<uint64_t, string>>& genomes){
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

void silo::partition(MetaStore &mdb, istream& in, const string& output_prefix_){
   cout << "Now partitioning fasta file to " << output_prefix_ << endl;
   vector<unique_ptr<ostream>> pid_to_ostream;
   const string output_prefix = output_prefix_ + '_';
   for(auto& x : mdb.pid_to_pango){
      auto out = make_unique<ofstream>(output_prefix + x + ".fasta");
      pid_to_ostream.emplace_back(std::move(out));
   }
   cout << "Created file streams for  " << output_prefix_ << endl;
   while (true) {
      string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      if(!mdb.epi_to_pid.contains(epi)) {
         // TODO logging
         continue;
      }

      auto pid = mdb.epi_to_pid.at(epi);
      *pid_to_ostream[pid] << epi_isl << endl << genome << endl;
   }
   cout << "Finished partitioning to " << output_prefix_ << endl;
}


//
// Created by Alexander Taepper on 16.11.22.
//

#include <silo/database.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>

void silo::Database::build(const std::string& part_prefix, const std::string& meta_suffix, const std::string& seq_suffix) {
   partitions.resize(part_def->partitions.size());
   tbb::blocked_range<size_t> r(0, part_def->partitions.size());
   tbb::parallel_for(r.begin(), r.end(), [&](size_t& i) {
      const auto& part = part_def->partitions[i];
      for (unsigned j = 0; j < part.chunks.size(); ++j) {
         std::string name = part_prefix + chunk_string(i, j);
         istream_wrapper seq_in(name + seq_suffix);
         std::ifstream meta_in(name + meta_suffix);
         std::cout << "Extending sequence-store from input file: " << name << std::endl;
         processSeq(partitions[i].seq_store, seq_in.get_is());
         processMeta(partitions[i].meta_store, meta_in, alias_key);
      }
   });
}

void silo::processSeq(silo::SequenceStore& seq_store, std::istream& in) {
   static constexpr unsigned interpretSize = 1024;

   uint32_t sid_ctr = seq_store.sequenceCount;
   std::vector<std::string> genomes;
   while (true) {
      std::string epi_isl, genome;
      if (!getline(in, epi_isl) || epi_isl.empty()) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         std::cerr << "length mismatch!" << std::endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      genomes.push_back(std::move(genome));
      if (genomes.size() >= interpretSize) {
         silo::interpret(seq_store, genomes);
         genomes.clear();
      }

      uint32_t sid = sid_ctr++;
      seq_store.epi_to_sid[epi] = sid;
      seq_store.sid_to_epi.push_back(epi);
   }
   silo::interpret(seq_store, genomes);
   db_info(seq_store, std::cout);
}

void silo::processMeta(MetaStore& mdb, std::istream& in, const std::unordered_map<std::string, std::string> alias_key) {
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   while (true) {
      std::string epi_isl, pango_lineage_raw, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage_raw, '\t')) break;
      if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;
      if (!getline(in, division, '\n')) break;

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolve_alias(alias_key, pango_lineage_raw);

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);

      silo::inputSequenceMeta(mdb, epi, pango_lineage, date, region, country, division);
   }

   if (mdb.sequence_count != mdb.epi_to_pid.size()) {
      std::cout << "ERROR: EPI is represented twice." << std::endl;
   }
}


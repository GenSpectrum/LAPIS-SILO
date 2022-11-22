//
// Created by Alexander Taepper on 16.11.22.
//

#include <silo/database.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>

void silo::Database::build(const std::string& part_prefix, const std::string& meta_suffix, const std::string& seq_suffix) {
   partitions.resize(part_def->partitions.size());
   tbb::blocked_range<size_t> r(0, part_def->partitions.size());
   tbb::parallel_for(r, [&](const decltype(r)& subr) {
      for (size_t i = subr.begin(), limit = subr.end(); i != limit; ++i) {
         const auto& part = part_def->partitions[i];
         for (unsigned j = 0; j < part.chunks.size(); ++j) {
            std::string name = part_prefix + chunk_string(i, j);
            istream_wrapper seq_in(name + seq_suffix);
            std::ifstream meta_in(name + meta_suffix);
            std::cout << "Extending sequence-store from input file: " << name << std::endl;
            unsigned count1 = processSeq(partitions[i].seq_store, seq_in.get_is());
            unsigned count2 = processMeta(partitions[i].meta_store, meta_in, alias_key);
            if (count1 != count2) {
               // Fatal error
               std::cerr << "Sequences in meta data and sequence data for chunk " << chunk_string(i, j) << " are not equal." << std::endl;
               std::cerr << "Abort build." << std::endl;
               partitions.clear();
               return;
            }
            partitions[i].sequenceCount += count1;
         }
      }
   });
}

unsigned silo::processSeq(silo::SequenceStore& seq_store, std::istream& in) {
   static constexpr unsigned interpretSize = 1024;

   unsigned sequence_count = 0;

   std::vector<std::string> genomes;
   while (true) {
      std::string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         std::cerr << "length mismatch!" << std::endl;
         throw std::runtime_error("length mismatch.");
      }

      genomes.push_back(std::move(genome));
      if (genomes.size() >= interpretSize) {
         seq_store.interpret(genomes);
         genomes.clear();
      }

      ++sequence_count;
   }
   seq_store.interpret(genomes);
   seq_store.db_info(std::cout);

   return sequence_count;
}

unsigned silo::processMeta(MetaStore& mdb, std::istream& in, const std::unordered_map<std::string, std::string>& alias_key) {
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   unsigned sequence_count = 0;

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
      ++sequence_count;
   }

   return sequence_count;
}

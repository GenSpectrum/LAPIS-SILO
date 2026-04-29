#pragma once

#include <algorithm>
#include <filesystem>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "silo/append/ndjson_line_reader.h"
#include "silo/common/phylo_tree.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/initialize/initializer.h"
#include "silo/storage/reference_genomes.h"

// Header-only utilities shared between performance benchmarks.
// All definitions live inside an anonymous namespace so that each benchmark
// translation unit gets its own copy without ODR conflicts.

namespace {

// --- Filesystem helpers ---

void changeCwdToTestFolder() {
   size_t search_depth = 4;
   std::filesystem::path candidate = std::filesystem::current_path();
   for (size_t i = 0; i < search_depth; ++i, candidate = candidate / "..") {
      if (std::filesystem::exists(candidate / "testBaseData/exampleDataset")) {
         std::filesystem::current_path(candidate);
         return;
      }
   }
   throw std::runtime_error(fmt::format(
      "Should be run from the repository root; could not find it from {}",
      std::filesystem::current_path().string()
   ));
}

std::string readReferenceFromFile() {
   auto reference_genomes =
      silo::ReferenceGenomes::readFromFile("testBaseData/exampleDataset/reference_genomes.json");
   if (reference_genomes.raw_nucleotide_sequences.empty()) {
      throw std::runtime_error("No nucleotide sequences found in reference_genomes.json");
   }
   return reference_genomes.raw_nucleotide_sequences.at(0);
}

// --- Sequence evolution model ---

constexpr double DEFAULT_MUTATION_RATE = 0.001;
constexpr double DEFAULT_DEATH_RATE = 0.1;
constexpr size_t DEFAULT_GENERATIONS = 5;
constexpr size_t DEFAULT_CHILDREN_PER_NODE = 3;

class SequenceTreeGenerator {
   std::mt19937 rng;
   const std::string& reference;
   double mutation_rate;
   double death_rate;
   size_t generations;
   size_t children_per_node;

   char mutateBase(char base) {
      std::uniform_int_distribution<size_t> dist(0, 3);
      char new_base;
      do {
         silo::Nucleotide::Symbol new_symbol = silo::Nucleotide::SYMBOLS.at(dist(rng));
         new_base = silo::Nucleotide::symbolToChar(new_symbol);
      } while (new_base == base);
      return new_base;
   }

   std::string mutateSequence(std::string_view sequence) {
      std::string mutated{sequence};
      std::binomial_distribution<size_t> num_mutations_dist(sequence.size(), mutation_rate);
      const size_t num_mutations = num_mutations_dist(rng);
      std::uniform_int_distribution<size_t> pos_dist(0, sequence.size() - 1);
      for (size_t i = 0; i < num_mutations; ++i) {
         const size_t pos = pos_dist(rng);
         mutated[pos] = mutateBase(mutated[pos]);
      }
      return mutated;
   }

  public:
   SequenceTreeGenerator(
      const std::string& ref,
      uint64_t seed = 42,
      double mut_rate = DEFAULT_MUTATION_RATE,
      double death = DEFAULT_DEATH_RATE,
      size_t gens = DEFAULT_GENERATIONS,
      size_t children = DEFAULT_CHILDREN_PER_NODE
   )
       : rng(seed),
         reference(ref),
         mutation_rate(mut_rate),
         death_rate(death),
         generations(gens),
         children_per_node(children) {}

   std::vector<std::string> generateEvolvedSequences() {
      std::vector<std::string> all_sequences = {reference};
      std::vector<size_t> current_gen = {0};
      std::bernoulli_distribution survives(1.0 - death_rate);
      for (size_t gen = 0; gen < generations; ++gen) {
         std::vector<size_t> next_gen;
         for (size_t seq_index : current_gen) {
            for (size_t child = 0; child < children_per_node; ++child) {
               if (survives(rng)) {
                  all_sequences.push_back(mutateSequence(all_sequences.at(seq_index)));
                  next_gen.push_back(all_sequences.size() - 1);
               }
            }
         }
         if (next_gen.empty()) {
            next_gen.push_back(all_sequences.size() - 1);
         }
         current_gen = std::move(next_gen);
      }
      return all_sequences;
   }
};

// --- Short-read generation ---

constexpr size_t DEFAULT_READ_COUNT = 5'000'000;
constexpr size_t DEFAULT_READ_LENGTH = 200;

struct ShortRead {
   size_t id;
   size_t offset;
   std::string sequence;
};

class ShortReadGenerator {
   std::vector<std::string> evolved_sequences;
   std::mt19937 rng;
   std::uniform_int_distribution<size_t> seq_dist;
   size_t count;
   size_t read_length;
   size_t num_positions;

  public:
   class iterator {
      ShortReadGenerator* generator;
      size_t current_id;

     public:
      using iterator_category = std::input_iterator_tag;
      using value_type = ShortRead;
      using difference_type = std::ptrdiff_t;

      iterator(ShortReadGenerator* gen, size_t id)
          : generator(gen),
            current_id(id) {}

      ShortRead operator*() { return generator->generateAt(current_id); }
      iterator& operator++() {
         ++current_id;
         return *this;
      }
      iterator operator++(int) {
         iterator tmp = *this;
         ++current_id;
         return tmp;
      }
      bool operator==(const iterator& other) const { return current_id == other.current_id; }
      bool operator!=(const iterator& other) const { return current_id != other.current_id; }
   };

   ShortReadGenerator(
      const std::string& reference,
      size_t count,
      size_t read_length,
      uint64_t seed = 42
   )
       : count(count),
         read_length(read_length) {
      if (read_length > reference.size()) {
         throw std::invalid_argument(fmt::format(
            "read_length ({}) exceeds reference length ({})", read_length, reference.size()
         ));
      }
      SequenceTreeGenerator tree_gen(reference, seed);
      evolved_sequences = tree_gen.generateEvolvedSequences();
      SPDLOG_INFO("Generated {} evolved sequences from tree model", evolved_sequences.size());
      num_positions = reference.size() - read_length + 1;
      rng.seed(seed + 1000);
      seq_dist = std::uniform_int_distribution<size_t>(0, evolved_sequences.size() - 1);
   }

   ShortRead generateAt(size_t read_id) {
      const size_t offset = (read_id * num_positions) / count;
      const auto& source_seq = evolved_sequences[seq_dist(rng)];
      return {read_id, offset, source_seq.substr(offset, read_length)};
   }

   iterator begin() { return iterator(this, 0); }
   iterator end() { return iterator(this, count); }
   [[nodiscard]] size_t size() const { return count; }
};

// --- NDJSON generators ---

std::stringstream generateShortReadNdjson(
   const std::string& reference,
   size_t count = DEFAULT_READ_COUNT,
   size_t read_length = DEFAULT_READ_LENGTH
) {
   ShortReadGenerator generator(reference, count, read_length);
   std::stringstream buffer;
   for (const auto& read : generator) {
      buffer << fmt::format(
         R"({{"readId":"read_{}","samplingDate":"2024-01-01","locationName":"generated","main":{{"insertions":[],"offset":{},"sequence":"{}"}}}})",
         read.id,
         read.offset,
         read.sequence
      ) << "\n";
   }
   return buffer;
}

constexpr size_t DEFAULT_FULL_SEQ_COUNT = 100'000;

std::stringstream generateFullSequenceNdjson(
   const std::string& reference,
   size_t count = DEFAULT_FULL_SEQ_COUNT
) {
   SequenceTreeGenerator tree_gen(reference);
   const auto evolved = tree_gen.generateEvolvedSequences();
   SPDLOG_INFO(
      "Repeating {} evolved sequences to fill {} full-sequence entries",
      evolved.size(),
      count
   );
   std::stringstream buffer;
   for (size_t i = 0; i < count; ++i) {
      const auto& seq = evolved[i % evolved.size()];
      buffer << fmt::format(
         R"({{"key":"{}","main":{{"sequence":"{}","insertions":[]}}}})", i, seq
      ) << "\n";
   }
   return buffer;
}

// --- Database initializers ---

std::shared_ptr<silo::Database> initializeDatabaseWithShortReadSchema(
   const std::string& reference
) {
   auto database_config = silo::config::DatabaseConfig::getValidatedConfig(R"(
schema:
  instanceName: test
  metadata:
    - name: readId
      type: string
    - name: samplingDate
      type: date
    - name: locationName
      type: string
  primaryKey: readId
)");
   silo::ReferenceGenomes reference_genomes{{{"main", reference}}, {}};
   auto database = std::make_shared<silo::Database>();
   database->createTable(
      silo::schema::TableName::getDefault(),
      silo::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         std::move(reference_genomes),
         {},
         silo::common::PhyloTree{},
         /*without_unaligned_sequences=*/true
      )
   );
   return database;
}

std::shared_ptr<silo::Database> initializeDatabaseWithFullSequenceSchema(
   const std::string& reference
) {
   auto database_config = silo::config::DatabaseConfig::getValidatedConfig(R"(
schema:
  instanceName: test
  metadata:
    - name: key
      type: string
  primaryKey: key
)");
   silo::ReferenceGenomes reference_genomes{{{"main", reference}}, {}};
   auto database = std::make_shared<silo::Database>();
   database->createTable(
      silo::schema::TableName::getDefault(),
      silo::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         std::move(reference_genomes),
         {},
         silo::common::PhyloTree{},
         /*without_unaligned_sequences=*/true
      )
   );
   return database;
}

}  // namespace

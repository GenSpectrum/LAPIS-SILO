#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

#include <arrow/compute/initialize.h>
#include <spdlog/spdlog.h>

#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/query.h"
#include "silo/storage/reference_genomes.h"

using silo::query_engine::Query;
using silo::Database;

namespace {

constexpr size_t DEFAULT_READ_COUNT = 5'000'000;
constexpr size_t DEFAULT_READ_LENGTH = 200;
constexpr double DEFAULT_MUTATION_RATE = 0.001;
constexpr double DEFAULT_DEATH_RATE = 0.1;
constexpr size_t DEFAULT_GENERATIONS = 5;
constexpr size_t DEFAULT_CHILDREN_PER_NODE = 3;
constexpr size_t DEFAULT_QUERY_COUNT = 10'000;

void changeCwdToTestFolder() {
   // Look for the test data directory (`testBaseData`) in the current directory and up to
   // <search_depth> directories above the current directory. If found, change the current working
   // directory to the directory containing the test data directory
   size_t search_depth = 4;
   std::filesystem::path candidate_directory = std::filesystem::current_path().string();
   for (size_t i = 0; i < search_depth; i++, candidate_directory = candidate_directory / "..") {
      if (std::filesystem::exists(candidate_directory / "testBaseData/exampleDataset")) {
         std::filesystem::current_path(candidate_directory);
         return;
      }
   }
   throw std::runtime_error(fmt::format(
      "Should be run in root of repository, got {} and could not find root by heuristics",
      std::filesystem::current_path().string()
   ));
}

std::string readReferenceFromFile() {
   auto reference_genomes =
      silo::ReferenceGenomes::readFromFile("testBaseData/exampleDataset/reference_genomes.json");
   if (reference_genomes.raw_nucleotide_sequences.empty()) {
      throw std::runtime_error("No nucleotide sequences found in reference genomes file");
   }
   return reference_genomes.raw_nucleotide_sequences.at(0);
}

using silo::Nucleotide;

// Simple tree-based sequence evolution model
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
         Nucleotide::Symbol new_symbol = Nucleotide::SYMBOLS.at(dist(rng));
         new_base = Nucleotide::symbolToChar(new_symbol);
      } while (new_base == base);
      return new_base;
   }

   std::string mutateSequence(std::string_view sequence) {
      std::string mutated{sequence};
      const size_t seq_length = sequence.size();

      // Sample the number of mutations from a binomial distribution
      std::binomial_distribution<size_t> num_mutations_dist(seq_length, mutation_rate);
      size_t num_mutations = num_mutations_dist(rng);

      // Randomly choose positions to mutate
      std::uniform_int_distribution<size_t> pos_dist(0, seq_length - 1);
      for (size_t i = 0; i < num_mutations; ++i) {
         size_t pos = pos_dist(rng);
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

   // Generate evolved sequences using a tree model
   std::vector<std::string> generateEvolvedSequences() {
      std::vector<std::string> all_generated_sequences = {reference};
      std::vector<std::string_view> current_generation = {reference};
      std::vector<std::string_view> next_generation;
      std::bernoulli_distribution survives(1.0 - death_rate);

      for (size_t gen = 0; gen < generations; ++gen) {
         next_generation.clear();
         for (const auto& seq : current_generation) {
            for (size_t child = 0; child < children_per_node; ++child) {
               if (survives(rng)) {
                  all_generated_sequences.push_back(mutateSequence(seq));
                  next_generation.push_back(all_generated_sequences.back());
               }
            }
         }
         if (next_generation.empty()) {
            // If all died, keep at least one survivor
            next_generation.push_back(all_generated_sequences.back());
         }
         current_generation = std::move(next_generation);
      }
      return all_generated_sequences;
   }
};

struct ShortRead {
   size_t id;
   size_t offset;
   std::string sequence;
};

// Lazy generator for short reads - generates on-demand without materializing all reads
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
      using pointer = const ShortRead*;
      using reference = ShortRead;

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
      SequenceTreeGenerator tree_gen(reference, seed);
      evolved_sequences = tree_gen.generateEvolvedSequences();

      SPDLOG_INFO("Generated {} evolved sequences from tree model", evolved_sequences.size());

      const size_t seq_length = reference.size();
      SILO_ASSERT(read_length < seq_length);
      num_positions = seq_length - read_length + 1;

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

std::shared_ptr<Database> initializeDatabaseWithSingleReference(std::string reference) {
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

struct TestDatabaseResult {
   std::shared_ptr<Database> database;
   size_t reference_length;
};

TestDatabaseResult setupTestDatabase() {
   std::string reference = readReferenceFromFile();
   SPDLOG_INFO("Read reference sequence of length {}", reference.size());
   size_t ref_length = reference.size();

   auto input_buffer = generateShortReadNdjson(reference);
   SPDLOG_INFO("Generated short read NDJSON data");

   auto database = initializeDatabaseWithSingleReference(reference);

   auto input_data_stream = silo::append::NdjsonLineReader{input_buffer};
   database->appendData(silo::schema::TableName::getDefault(), input_buffer);

   return {database, ref_length};
}

class QueryGenerator {
   std::mt19937 rng;
   size_t reference_length;
   size_t query_counter = 0;
   static constexpr std::array<char, 5> SYMBOLS = {'A', 'C', 'G', 'T', '-'};

  public:
   QueryGenerator(size_t ref_length, uint64_t seed = 42)
       : rng(seed),
         reference_length(ref_length) {}

   std::string generateQuery() {
      std::uniform_int_distribution<size_t> pos_dist(1, reference_length - 1);
      size_t position = pos_dist(rng);

      bool use_all_symbols = (query_counter++ % 2 == 1);

      if (use_all_symbols) {
         // Query all 5 symbols (A, C, G, T, -) at the same position in an OR
         return fmt::format(
            R"({{"action":{{"type":"Aggregated"}},"filterExpression":{{"children":[{{"children":[{{"children":[{{"column":"locationName","value":"generated","type":"StringEquals"}}],"type":"Or"}},{{"column":"samplingDate","from":"2024-01-01","to":"2024-01-07","type":"DateBetween"}}],"type":"And"}},{{"children":[{{"position":{0},"symbol":"A","type":"NucleotideEquals"}},{{"position":{0},"symbol":"C","type":"NucleotideEquals"}},{{"position":{0},"symbol":"G","type":"NucleotideEquals"}},{{"position":{0},"symbol":"T","type":"NucleotideEquals"}},{{"position":{0},"symbol":"-","type":"NucleotideEquals"}}],"type":"Or"}},{{"column":"samplingDate","from":"2024-01-01","to":"2024-01-07","type":"DateBetween"}}],"type":"And"}}}})",
            position
         );
      }
      // Query a single random symbol at the position
      std::uniform_int_distribution<size_t> sym_dist(0, SYMBOLS.size() - 1);
      char symbol = SYMBOLS[sym_dist(rng)];
      return fmt::format(
         R"({{"action":{{"type":"Aggregated"}},"filterExpression":{{"children":[{{"children":[{{"children":[{{"column":"locationName","value":"generated","type":"StringEquals"}}],"type":"Or"}},{{"column":"samplingDate","from":"2024-01-01","to":"2024-01-07","type":"DateBetween"}}],"type":"And"}},{{"position":{},"symbol":"{}","type":"NucleotideEquals"}},{{"column":"samplingDate","from":"2024-01-01","to":"2024-01-07","type":"DateBetween"}}],"type":"And"}}}})",
         position,
         symbol
      );
   }
};

void executeAllQueries(
   const std::shared_ptr<Database>& database,
   size_t reference_length,
   size_t query_count = DEFAULT_QUERY_COUNT
) {
   QueryGenerator query_gen(reference_length);

   for (size_t query_num = 1; query_num <= query_count; ++query_num) {
      if (query_num % 1000 == 0) {
         SPDLOG_INFO("Executing query number {}", query_num);
      }
      std::string query_string = query_gen.generateQuery();
      auto query = Query::parseQuery(query_string);
      auto query_plan = database->createQueryPlan(*query, {}, "test_query");
      std::ofstream null_output("/dev/null");
      silo::query_engine::exec_node::NdjsonSink sink{&null_output, query_plan.results_schema};
      query_plan.executeAndWrite(sink, /*timeout_in_seconds=*/20);
   }
}

void run() {
   changeCwdToTestFolder();
   SILO_ASSERT(arrow::compute::Initialize().ok());
   SPDLOG_INFO("Building database for benchmark:");

   auto [database, reference_length] = setupTestDatabase();

   while(true){
      SPDLOG_INFO("Starting full query set benchmark ({} queries):", DEFAULT_QUERY_COUNT);
      auto start = std::chrono::high_resolution_clock::now();
      executeAllQueries(database, reference_length);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      SPDLOG_INFO("Finished full query set in {}.{:03} seconds", duration / 1000, duration % 1000);
   }
}

}  // namespace

int main(){
   try {
      run();
   } catch (std::exception& e) {
      SPDLOG_ERROR(e.what());
      return EXIT_FAILURE;
   }
}

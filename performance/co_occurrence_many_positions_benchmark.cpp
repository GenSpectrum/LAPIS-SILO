// Measurement-only benchmark for a co-occurrence groupBy over MANY (>100) sequence positions --
// the regime the bitmap-aggregation node's whole-chunk fast paths target.
//
// The reference is length 128 and the dataset is full-coverage (every sequence spans the whole
// reference, with no N and no null). Mutations occur only at a small set of "variable" positions;
// every other position is structurally monomorphic (never mutated). Grouping on all 128 positions
// therefore collapses the ~118 monomorphic dimensions to a single "all reference" group per 2^16
// chunk, while the handful of variable positions carry the actual combinations. This mirrors real
// genomic data (a conserved genome with a few variable sites) and stresses exactly the per-chunk
// grouping / whole-chunk-label path.
//
// There is deliberately no before/after comparison in here: run this benchmark on a branch WITHOUT
// the per-chunk bitmap-aggregation optimization to get the baseline number, and again WITH it to
// get the optimized number. Because the query and data are identical, the two runs are directly
// comparable.

#include <algorithm>
#include <array>
#include <chrono>
#include <iterator>
#include <memory>
#include <random>
#include <sstream>
#include <string>

#include <arrow/compute/api.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "sequence_generator.h"
#include "rhydb/config/database_config.h"
#include "rhydb/config/runtime_config.h"
#include "rhydb/database.h"
#include "rhydb/initialize/initializer.h"
#include "rhydb/query_engine/exec_node/ndjson_sink.h"
#include "rhydb/query_engine/planner.h"
#include "rhydb/query_engine/query_plan.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/reference_genomes.h"

namespace {

using rhydb::Database;
using rhydb::config::QueryOptions;
using rhydb::query_engine::Planner;

// --- Benchmark parameters ---

constexpr size_t REFERENCE_LENGTH = 128;  // grouped on all 128 positions (1-based), i.e. > 100
constexpr size_t NUM_SEQUENCES = 2'000'000;
constexpr double VARIABLE_MUTATION_RATE = 0.15;
constexpr int ITERATIONS = 3;

// The only 1-based positions that ever mutate; every other position stays at the reference symbol
// for every sequence, so its grouping dimension is monomorphic across each whole 2^16 chunk.
constexpr std::array<size_t, 10> VARIABLE_POSITIONS{7, 19, 31, 43, 55, 67, 79, 91, 103, 115};

std::string makeReference() {
   constexpr std::array<char, 4> bases{'A', 'C', 'G', 'T'};
   std::mt19937 rng{42};
   std::uniform_int_distribution<size_t> base_dist(0, bases.size() - 1);
   std::string reference(REFERENCE_LENGTH, 'A');
   for (char& base : reference) {
      base = bases.at(base_dist(rng));
   }
   return reference;
}

std::string makeNdjson(const std::string& reference) {
   constexpr std::array<char, 4> bases{'A', 'C', 'G', 'T'};
   std::mt19937 rng{1234};
   std::uniform_int_distribution<size_t> base_dist(0, bases.size() - 1);
   std::uniform_real_distribution<double> probability(0.0, 1.0);

   std::string out;
   out.reserve(NUM_SEQUENCES * (REFERENCE_LENGTH + 48));
   std::string sequence;
   for (size_t row = 0; row < NUM_SEQUENCES; ++row) {
      sequence.assign(reference);
      for (const size_t position : VARIABLE_POSITIONS) {
         if (probability(rng) < VARIABLE_MUTATION_RATE) {
            sequence[position - 1] = bases.at(base_dist(rng));
         }
      }
      fmt::format_to(
         std::back_inserter(out),
         "{{\"primaryKey\":\"id_{}\",\"main\":{{\"sequence\":\"{}\",\"insertions\":[]}}}}\n",
         row,
         sequence
      );
   }
   return out;
}

std::shared_ptr<Database> setupDatabase(const std::string& reference, const std::string& ndjson) {
   auto database_config = rhydb::config::DatabaseConfig::getValidatedConfig(R"(
schema:
  instanceName: co_occurrence_many_positions_benchmark
  metadata:
    - name: primaryKey
      type: string
  primaryKey: primaryKey
)");

   rhydb::ReferenceGenomes reference_genomes{{{"main", reference}}, {}};

   auto database = std::make_shared<Database>();
   database->createTable(
      rhydb::schema::TableName::getDefault(),
      rhydb::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         reference_genomes,
         {},
         rhydb::common::PhyloTree{},
         /*without_unaligned_sequences=*/true
      )
   );

   std::istringstream ndjson_stream{ndjson};
   database->appendData(rhydb::schema::TableName::getDefault(), ndjson_stream);
   return database;
}

std::string buildQuery() {
   std::string assignments;
   std::string group_keys;
   for (size_t position = 1; position <= REFERENCE_LENGTH; ++position) {
      if (position > 1) {
         assignments += ", ";
         group_keys += ", ";
      }
      assignments += fmt::format("s{} := main.at({})", position, position);
      group_keys += fmt::format("s{}", position);
   }
   return fmt::format(
      "default.map({{{}}}).groupBy({{count:=count()}}, {{{}}})", assignments, group_keys
   );
}

/// Plan and execute `query` through the regular Planner, returning the number of result rows.
size_t planAndExecute(
   const std::string& query,
   const std::shared_ptr<Database>& database,
   const QueryOptions& query_options
) {
   auto query_plan = Planner::planSaneqlQuery(query, database->tables, query_options, "bench");
   std::stringstream result;
   rhydb::query_engine::exec_node::NdjsonSink sink{&result, query_plan.results_schema};
   query_plan.executeAndWrite(sink, /*timeout_in_seconds=*/600);

   size_t rows = 0;
   std::stringstream stream{result.str()};
   std::string line;
   while (std::getline(stream, line)) {
      if (!line.empty()) {
         ++rows;
      }
   }
   return rows;
}

}  // namespace

int main() {
   changeCwdToTestFolder();
   // Register Arrow's compute kernels (e.g. utf8_slice_codeunits, used by the `at` scalar
   // function).
   if (!arrow::compute::Initialize().ok()) {
      SPDLOG_ERROR("Failed to initialize Arrow compute");
      return 1;
   }

   const auto query_options = rhydb::config::RuntimeConfig::withDefaults().query_options;

   SPDLOG_INFO(
      "Co-occurrence (many positions) benchmark: {} sequences, reference length {}, grouping on "
      "all "
      "{} positions, {} of them variable at rate {}",
      NUM_SEQUENCES,
      REFERENCE_LENGTH,
      REFERENCE_LENGTH,
      VARIABLE_POSITIONS.size(),
      VARIABLE_MUTATION_RATE
   );

   const std::string reference = makeReference();
   const std::string ndjson = makeNdjson(reference);

   const auto setup_start = std::chrono::high_resolution_clock::now();
   auto database = setupDatabase(reference, ndjson);
   const auto setup_end = std::chrono::high_resolution_clock::now();
   SPDLOG_INFO(
      "Database setup in {:.2f} s", std::chrono::duration<double>(setup_end - setup_start).count()
   );

   const std::string query = buildQuery();

   double sum_ms = 0;
   double min_ms = 0;
   size_t result_rows = 0;
   for (int i = 0; i < ITERATIONS; ++i) {
      const auto start = std::chrono::high_resolution_clock::now();
      result_rows = planAndExecute(query, database, query_options);
      const auto end = std::chrono::high_resolution_clock::now();
      const double ms = std::chrono::duration<double, std::milli>(end - start).count();
      sum_ms += ms;
      min_ms = (i == 0) ? ms : std::min(min_ms, ms);
   }

   SPDLOG_INFO("Result rows: {}", result_rows);
   SPDLOG_INFO(
      "Query execution over {} iterations: avg {:.1f} ms, min {:.1f} ms",
      ITERATIONS,
      sum_ms / ITERATIONS,
      min_ms
   );

   return 0;
}

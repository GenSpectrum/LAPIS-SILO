// Measurement-only benchmark for a co-occurrence groupBy over the real SARS-CoV-2 mutation
// positions in performance/mutations.csv, run against real sr2silo short-read data.
//
// The dataset (`sorted.ndjson.zst`) is already in SILO's flat ingest format (each read is a
// `main:{sequence,insertions,offset}` object), so it is fed straight to appendData -- no
// transformer. Because these are short reads, each read covers only a small window of the genome,
// so most of the ~141 grouped positions are "not covered" for any given read: this exercises the
// coverage-based grouping (all-missing / general missing) on realistic data.
//
// Two phases, decoupled via SILO's on-disk state so the query can be profiled without paying for
// ingestion every run. Set REAL_DB_DIR:
//   * if it holds a saved database    -> load it and run the query (fast; profile this);
//   * otherwise                       -> read flat NDJSON from stdin, ingest, and save it there.
// Ingest:  zstd -dc ~/sorted.ndjson.zst | head -n N | REAL_DB_DIR=/path
// ./real_data_mutations_benchmark Query:   REAL_DB_DIR=/path ./real_data_mutations_benchmark

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <arrow/compute/api.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "sequence_generator.h"
#include "rhydb/append/table_inserter.h"
#include "rhydb/common/input_stream_wrapper.h"
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

constexpr int ITERATIONS = 5;

// Every maximal run of digits in mutations.csv is a 1-based position (e.g. "C21T" -> 21).
std::vector<uint32_t> readMutationPositions(const std::string& path) {
   std::ifstream input{path};
   if (!input) {
      throw std::runtime_error(fmt::format("could not open {}", path));
   }
   const std::string content{
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()
   };
   std::vector<uint32_t> positions;
   for (size_t i = 0; i < content.size();) {
      if (std::isdigit(static_cast<unsigned char>(content[i])) == 0) {
         ++i;
         continue;
      }
      uint32_t value = 0;
      while (i < content.size() && std::isdigit(static_cast<unsigned char>(content[i])) != 0) {
         value = (value * 10) + static_cast<uint32_t>(content[i] - '0');
         ++i;
      }
      positions.push_back(value);
   }
   return positions;
}

std::string buildQuery(const std::vector<uint32_t>& positions) {
   std::string assignments;
   std::string group_keys;
   for (size_t i = 0; i < positions.size(); ++i) {
      if (i > 0) {
         assignments += ", ";
         group_keys += ", ";
      }
      assignments += fmt::format("s{} := main.at({})", i, positions.at(i));
      group_keys += fmt::format("s{}", i);
   }
   return fmt::format(
      "default.map({{{}}}).groupBy({{count:=count()}}, {{{}}})", assignments, group_keys
   );
}

Database makeEmptyDatabase() {
   auto database_config = rhydb::config::DatabaseConfig::getValidatedConfig(R"(
schema:
  instanceName: real_data_mutations_benchmark
  metadata:
    - name: readId
      type: string
  primaryKey: readId
)");

   // Only the `main` nucleotide sequence is needed for the mutation query; the (real) SARS-CoV-2
   // reference is the one bundled with the example dataset.
   rhydb::ReferenceGenomes reference_genomes{{{"main", readReferenceFromFile()}}, {}};

   Database database;
   database.createTable(
      rhydb::schema::TableName::getDefault(),
      rhydb::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         reference_genomes,
         {},
         rhydb::common::PhyloTree{},
         /*without_unaligned_sequences=*/true
      )
   );
   return database;
}

// Ingests every record. `ndjson_path` may be a plain .ndjson or a .zst archive --
// InputStreamWrapper decompresses on the fly, so nothing is written uncompressed to disk. Empty
// path -> read stdin.
Database ingest(const std::string& ndjson_path) {
   Database database = makeEmptyDatabase();

   // CLUSTER (env) enables coverage-driven clustered buffering on the `main` column, so reads that
   // cover the same genome window land in the same 2^16 chunk -- the input is already position
   // sorted, so this checks whether clustering buys anything on top of that.
   rhydb::append::ClusteredBufferingOptions clustering;
   if (std::getenv("CLUSTER") != nullptr) {
      clustering.enabled = true;
      clustering.driver_column_name = "main";
      SPDLOG_INFO(
         "Clustered buffering ENABLED (driver=main, num_buffers={}, span_growth={})",
         clustering.num_buffers,
         clustering.span_growth_threshold_fraction
      );
   }

   const auto start = std::chrono::high_resolution_clock::now();
   if (ndjson_path.empty()) {
      database.appendData(rhydb::schema::TableName::getDefault(), std::cin, clustering);
   } else {
      const rhydb::InputStreamWrapper input{std::filesystem::path{ndjson_path}};
      database.appendData(
         rhydb::schema::TableName::getDefault(), input.getInputStream(), clustering
      );
   }
   const auto end = std::chrono::high_resolution_clock::now();
   SPDLOG_INFO(
      "Ingested {} in {:.2f} s",
      ndjson_path.empty() ? "stdin" : ndjson_path,
      std::chrono::duration<double>(end - start).count()
   );
   return database;
}

/// Plan and execute `query` through the regular Planner, returning the number of result rows.
size_t planAndExecute(
   const std::string& query,
   const Database& database,
   const QueryOptions& query_options
) {
   auto query_plan = Planner::planSaneqlQuery(query, database.tables, query_options, "bench");
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
   if (!arrow::compute::Initialize().ok()) {
      SPDLOG_ERROR("Failed to initialize Arrow compute");
      return 1;
   }

   const auto query_options = rhydb::config::RuntimeConfig::withDefaults().query_options;

   const auto positions = readMutationPositions("performance/mutations.csv");
   SPDLOG_INFO("Loaded {} mutation positions from performance/mutations.csv", positions.size());

   const char* db_dir_env = std::getenv("REAL_DB_DIR");
   const std::filesystem::path db_dir = db_dir_env != nullptr ? db_dir_env : std::string{};
   const bool have_saved_db =
      !db_dir.empty() && std::filesystem::exists(db_dir) && !std::filesystem::is_empty(db_dir);

   std::optional<Database> database;
   if (have_saved_db) {
      const auto start = std::chrono::high_resolution_clock::now();
      database = Database::loadDatabaseStateFromPath(db_dir);
      const auto end = std::chrono::high_resolution_clock::now();
      if (!database.has_value()) {
         SPDLOG_ERROR("Failed to load database from {}", db_dir.string());
         return 1;
      }
      SPDLOG_INFO(
         "Loaded database from {} in {:.2f} s",
         db_dir.string(),
         std::chrono::duration<double>(end - start).count()
      );
   } else {
      const char* ndjson_env = std::getenv("REAL_NDJSON");
      database = ingest(ndjson_env != nullptr ? ndjson_env : std::string{});
      if (!db_dir.empty()) {
         database->saveDatabaseState(db_dir);
         SPDLOG_INFO("Saved database to {}", db_dir.string());
      }
   }

   const std::string query = buildQuery(positions);

   double sum_ms = 0;
   double min_ms = 0;
   size_t result_rows = 0;
   for (int i = 0; i < ITERATIONS; ++i) {
      const auto start = std::chrono::high_resolution_clock::now();
      result_rows = planAndExecute(query, database.value(), query_options);
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

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <fmt/format.h>

#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/query.h"

namespace {

using silo::Database;
using silo::query_engine::Query;
using silo::query_engine::actions::Aggregated;
using silo::query_engine::filter::expressions::Expression;
using silo::query_engine::filter::expressions::ExpressionVector;
using silo::query_engine::filter::expressions::Or;
using silo::query_engine::filter::expressions::StringEquals;
using silo::query_engine::filter::expressions::StringInSet;
using silo::query_engine::filter::expressions::True;

std::shared_ptr<Database> initializeDatabase() {
   auto database_config = silo::config::DatabaseConfig::getValidatedConfig(R"(
schema:
  instanceName: test
  metadata:
    - name: accession
      type: string
    - name: country
      type: string
      generateIndex: true
  primaryKey: accession
)");

   silo::ReferenceGenomes reference_genomes{{}, {}};

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

/// Create a database with `num_records` records, each with a unique accession
std::shared_ptr<Database> setupTestDatabase(size_t num_records) {
   std::stringstream input_buffer;

   std::vector<std::string> countries = {"USA", "Germany", "France", "UK", "China", "Japan"};

   for (size_t i = 0; i < num_records; ++i) {
      std::string accession = fmt::format("ACC{:06}", i);
      std::string country = countries[i % countries.size()];
      input_buffer << fmt::format(
         R"({{"accession":"{}","country":"{}"}}
)",
         accession,
         country
      );
   }

   auto database = initializeDatabase();
   database->appendData(silo::schema::TableName::getDefault(), input_buffer);

   return database;
}

/// Build an OR expression with many StringEquals clauses
std::unique_ptr<Expression> buildManyStringEquals(
   const std::string& column,
   const std::vector<std::string>& values
) {
   ExpressionVector children;
   children.reserve(values.size());
   for (const auto& value : values) {
      children.push_back(std::make_unique<StringEquals>(column, value));
   }
   return std::make_unique<Or>(std::move(children));
}

/// Build a StringInSet expression
std::unique_ptr<Expression> buildStringInSet(
   const std::string& column,
   const std::vector<std::string>& values
) {
   std::unordered_set<std::string> value_set(values.begin(), values.end());
   return std::make_unique<StringInSet>(column, std::move(value_set));
}

void executeAggregatedQuery(const std::shared_ptr<Database>& database, Query& query) {
   auto query_plan = database->createQueryPlan(query, {}, "benchmark_query");
   std::stringstream result;
   query_plan.executeAndWrite(&result, /*timeout_in_seconds=*/60);
}

struct BenchmarkResult {
   double avg_ms;
   double min_ms;
   double max_ms;
};

template <typename FilterBuilder>
BenchmarkResult runBenchmark(
   const std::shared_ptr<Database>& database,
   FilterBuilder build_filter,
   int iterations
) {
   std::vector<int64_t> durations;
   durations.reserve(iterations);

   for (int i = 0; i < iterations; ++i) {
      // Build a fresh filter for each iteration
      auto filter = build_filter();
      Query query{std::move(filter), std::make_unique<Aggregated>(std::vector<std::string>{})};

      auto start = std::chrono::high_resolution_clock::now();
      executeAggregatedQuery(database, query);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
      durations.push_back(duration);
   }

   // Calculate statistics
   int64_t sum = 0;
   int64_t min_val = durations[0];
   int64_t max_val = durations[0];
   for (int64_t d : durations) {
      sum += d;
      min_val = std::min(min_val, d);
      max_val = std::max(max_val, d);
   }

   return BenchmarkResult{
      .avg_ms = static_cast<double>(sum) / static_cast<double>(iterations) / 1000.0,
      .min_ms = static_cast<double>(min_val) / 1000.0,
      .max_ms = static_cast<double>(max_val) / 1000.0
   };
}

}  // namespace

int main() {
   SPDLOG_INFO("=== StringInSet vs Many StringEquals Performance Benchmark ===");
   SPDLOG_INFO("");

   constexpr size_t NUM_RECORDS = 100000;
   constexpr int ITERATIONS = 5;

   SPDLOG_INFO("Setting up test database with {} records...", NUM_RECORDS);
   auto start_setup = std::chrono::high_resolution_clock::now();
   auto database = setupTestDatabase(NUM_RECORDS);
   auto end_setup = std::chrono::high_resolution_clock::now();
   auto setup_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end_setup - start_setup).count();
   SPDLOG_INFO(
      "Database setup completed in {}.{:03} seconds", setup_duration / 1000, setup_duration % 1000
   );
   SPDLOG_INFO("");

   std::vector<size_t> test_sizes = {10, 100, 500, 1000, 5000};

   for (size_t num_values : test_sizes) {
      SPDLOG_INFO("--- Benchmark with {} search values ---", num_values);

      // Generate search values (mix of existing and non-existing)
      std::vector<std::string> search_values;
      search_values.reserve(num_values);
      for (size_t i = 0; i < num_values; ++i) {
         // Every 10th value doesn't exist in the database
         if (i % 10 == 9) {
            search_values.push_back(fmt::format("NOTEXIST{:06}", i));
         } else {
            // Pick values spread across the database
            size_t idx = (i * NUM_RECORDS / num_values) % NUM_RECORDS;
            search_values.push_back(fmt::format("ACC{:06}", idx));
         }
      }

      // Lambda builders that capture search_values
      auto build_or = [&]() { return buildManyStringEquals("accession", search_values); };
      auto build_set = [&]() { return buildStringInSet("accession", search_values); };

      // Run benchmarks
      auto or_result = runBenchmark(database, build_or, ITERATIONS);
      SPDLOG_INFO(
         "OR({} StringEquals): avg={:.2f}ms, min={:.2f}ms, max={:.2f}ms",
         num_values,
         or_result.avg_ms,
         or_result.min_ms,
         or_result.max_ms
      );

      auto set_result = runBenchmark(database, build_set, ITERATIONS);
      SPDLOG_INFO(
         "StringInSet({}):     avg={:.2f}ms, min={:.2f}ms, max={:.2f}ms",
         num_values,
         set_result.avg_ms,
         set_result.min_ms,
         set_result.max_ms
      );

      double speedup = or_result.avg_ms / set_result.avg_ms;
      SPDLOG_INFO("Speedup: {:.1f}x", speedup);

      SPDLOG_INFO("");
   }

   // Test on indexed column (country)
   SPDLOG_INFO("=== Testing on INDEXED column (country) ===");
   SPDLOG_INFO("");

   std::vector<std::string> all_countries = {"USA", "Germany", "France", "UK", "China", "Japan"};

   for (size_t num_values : test_sizes) {
      SPDLOG_INFO("--- Benchmark with {} country values (indexed column) ---", num_values);

      std::vector<std::string> search_countries;
      search_countries.reserve(num_values);
      for (size_t i = 0; i < num_values; ++i) {
         // Every 11th value takes a country from the list
         if (i % 11 == 0) {
            size_t idx = i % search_countries.size();
            search_countries.push_back(search_countries.at(idx));
         } else {
            // Pick values spread across the database
            search_countries.push_back(fmt::format("NOTEXIST{:06}", i));
         }
      }

      auto build_or = [&]() { return buildManyStringEquals("country", search_countries); };
      auto build_set = [&]() { return buildStringInSet("country", search_countries); };

      auto or_result = runBenchmark(database, build_or, ITERATIONS);
      SPDLOG_INFO(
         "OR({} StringEquals): avg={:.2f}ms, min={:.2f}ms, max={:.2f}ms",
         num_values,
         or_result.avg_ms,
         or_result.min_ms,
         or_result.max_ms
      );

      auto set_result = runBenchmark(database, build_set, ITERATIONS);
      SPDLOG_INFO(
         "StringInSet({}):     avg={:.2f}ms, min={:.2f}ms, max={:.2f}ms",
         num_values,
         set_result.avg_ms,
         set_result.min_ms,
         set_result.max_ms
      );

      double speedup = or_result.avg_ms / set_result.avg_ms;
      SPDLOG_INFO("Speedup: {:.1f}x", speedup);

      SPDLOG_INFO("");
   }

   SPDLOG_INFO("=== Benchmark Complete ===");
   return 0;
}

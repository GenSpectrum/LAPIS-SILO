#include <algorithm>
#include <exception>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

#include <arrow/compute/initialize.h>
#include <spdlog/spdlog.h>

#include "sequence_generator.h"
#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/action_query.h"
#include "silo/query_engine/planner.h"
#include "silo/query_engine/binder.h"
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/storage/reference_genomes.h"

using silo::query_engine::ActionQuery;
using silo::query_engine::Planner;
using silo::query_engine::Binder;
using silo::Database;

namespace {

constexpr size_t DEFAULT_QUERY_COUNT = 10'000;

struct TestDatabaseResult {
   std::shared_ptr<Database> database;
   size_t reference_length;
};

TestDatabaseResult setupTestDatabase() {
   std::string reference = readReferenceFromFile();
   SPDLOG_INFO("Read reference sequence of length {}", reference.size());
   const size_t ref_length = reference.size();

   auto input_buffer = generateShortReadNdjson(reference);
   SPDLOG_INFO("Generated short read NDJSON data");

   auto database = initializeDatabaseWithShortReadSchema(reference);
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
      const size_t position = pos_dist(rng);

      const bool use_all_symbols = (query_counter++ % 2 == 1);

      if (use_all_symbols) {
         return fmt::format(
            R"({{"action":{{"type":"Aggregated"}},"filterExpression":{{"children":[{{"children":[{{"children":[{{"column":"locationName","value":"generated","type":"StringEquals"}}],"type":"Or"}},{{"column":"samplingDate","from":"2024-01-01","to":"2024-01-07","type":"DateBetween"}}],"type":"And"}},{{"children":[{{"position":{0},"symbol":"A","type":"NucleotideEquals"}},{{"position":{0},"symbol":"C","type":"NucleotideEquals"}},{{"position":{0},"symbol":"G","type":"NucleotideEquals"}},{{"position":{0},"symbol":"T","type":"NucleotideEquals"}},{{"position":{0},"symbol":"-","type":"NucleotideEquals"}}],"type":"Or"}},{{"column":"samplingDate","from":"2024-01-01","to":"2024-01-07","type":"DateBetween"}}],"type":"And"}}}})",
            position
         );
      }
      std::uniform_int_distribution<size_t> sym_dist(0, SYMBOLS.size() - 1);
      const char symbol = SYMBOLS[sym_dist(rng)];
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
      auto query = ActionQuery::parseQuery(query_string);

      auto bound_query = Binder::bindQuery(std::move(query), database->tables);
      auto query_plan = Planner::planQuery(std::move(bound_query), database->tables, {}, "test_query");   std::stringstream result;

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

   while (true) {
      SPDLOG_INFO("Starting full query set benchmark ({} queries):", DEFAULT_QUERY_COUNT);
      auto start = std::chrono::high_resolution_clock::now();
      executeAllQueries(database, reference_length);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      SPDLOG_INFO("Finished full query set in {}.{:03} seconds", duration / 1000, duration % 1000);
   }
}

}  // namespace

int main() {
   try {
      run();
   } catch (std::exception& e) {
      SPDLOG_ERROR(e.what());
      return EXIT_FAILURE;
   }
}

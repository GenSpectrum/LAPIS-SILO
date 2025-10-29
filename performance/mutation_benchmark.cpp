#include <utility>

#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/query.h"

namespace {

using silo::query_engine::Query;
using silo::query_engine::filter::expressions::True;
using silo::query_engine::filter::expressions::Negation;
using silo::query_engine::filter::expressions::StringEquals;
using silo::Nucleotide;
using silo::query_engine::actions::Mutations;
using silo::Database;

std::shared_ptr<Database> initializeDatabaseWithSingleReference(std::string reference){
   auto database_config = silo::config::DatabaseConfig::getValidatedConfig(R"(
schema:
  instanceName: test
  metadata:
    - name: key
      type: string
  primaryKey: key
)");

   silo::ReferenceGenomes reference_genomes{{{"main", reference}}, {}};

   return std::make_shared<silo::Database>(
      silo::Database{silo::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         std::move(reference_genomes),
         {},
         silo::common::PhyloTree{},
         /*without_unaligned_sequences=*/true
      )}
   );
}

size_t current_id = 0;

void addThousandShortReads(std::stringstream& buffer, size_t offset){
   for(size_t i = 0; i < 1000; ++i){
      std::string sequence = "ACGT";
      buffer << fmt::format(R"({{"key":"{}","main":{{"sequence":"{}","offset":{},"insertions":[]}}}}
)", current_id++, sequence, offset);
   }
}

std::shared_ptr<Database> setupTestDatabase(){
   std::string pattern = "ACGT";
   std::string reference;
   reference.reserve(4000);
   for (int i = 0; i < 1000; ++i) {
      reference += pattern;
   }

   std::stringstream input_buffer;

   for(size_t i = 0; i < 1000; ++i){
      addThousandShortReads(input_buffer, 0);
   }
   for(size_t i = 0; i < 1000; ++i){
      addThousandShortReads(input_buffer, 4);
   }
   for(size_t i = 0; i < 100; ++i){
      addThousandShortReads(input_buffer, 99);
   }
   for(size_t i = 0; i < 100; ++i){
      addThousandShortReads(input_buffer, 100 + i);
   }
   for(size_t i = 0; i < 1000; ++i){
      addThousandShortReads(input_buffer, 2000);
   }

   auto database = initializeDatabaseWithSingleReference(reference);

   auto input_data_stream = silo::append::NdjsonLineReader{input_buffer};
   silo::append::appendDataToDatabase(*database, input_data_stream);

   return database;
}

void printClipped(const std::string& output){
   std::istringstream iss(output);
   std::string line;
   int line_count = 0;
   int total_lines = 0;

   // Count total lines
   std::istringstream counter(output);
   while (std::getline(counter, line)) {
      total_lines++;
   }

   // Print first 5 lines
   while (std::getline(iss, line) && line_count < 5) {
      SPDLOG_INFO("{}", line);
      line_count++;
   }

   if (total_lines > 5) {
      SPDLOG_INFO("... (total {} lines)", total_lines);
   }
}

void executeMutationsAllQuery(std::shared_ptr<Database> database){
   std::vector<std::string_view> all_fields{Mutations<Nucleotide>::VALID_FIELDS.begin(), Mutations<Nucleotide>::VALID_FIELDS.end()};

   Query query{std::make_unique<True>(), std::make_unique<Mutations<Nucleotide>>(std::vector<std::string>{"main"}, 0.05, std::move(all_fields))};

   auto query_plan = query.toQueryPlan(std::move(database), {}, "test_query");
   std::stringstream result;
   query_plan.executeAndWrite(&result, /*timeout_in_seconds=*/3);
   printClipped(result.str());
}

void executeMutationsAlmostAllQuery(std::shared_ptr<Database> database){
   std::vector<std::string_view> all_fields{Mutations<Nucleotide>::VALID_FIELDS.begin(), Mutations<Nucleotide>::VALID_FIELDS.end()};

   Query query{std::make_unique<Negation>(std::make_unique<StringEquals>("key", "3")), std::make_unique<Mutations<Nucleotide>>(std::vector<std::string>{"main"}, 0.05, std::move(all_fields))};

   auto query_plan = query.toQueryPlan(std::move(database), {}, "test_query");
   std::stringstream result;
   query_plan.executeAndWrite(&result, /*timeout_in_seconds=*/3);
   printClipped(result.str());
}
}  // namespace

int main(){
   SPDLOG_INFO("Starting micro benchmark:");

   auto start0 = std::chrono::high_resolution_clock::now();
   auto database = setupTestDatabase();
   auto end0 = std::chrono::high_resolution_clock::now();
   auto duration0 = std::chrono::duration_cast<std::chrono::milliseconds>(end0 - start0).count();
   SPDLOG_INFO("Added all data in {}.{:03} seconds", duration0 / 1000, duration0 % 1000);

   auto start1 = std::chrono::high_resolution_clock::now();
   executeMutationsAllQuery(database);
   auto end1 = std::chrono::high_resolution_clock::now();
   auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
   SPDLOG_INFO("Finished executeMutationsAllQuery in {}.{:03} seconds", duration1 / 1000, duration1 % 1000);

   auto start2 = std::chrono::high_resolution_clock::now();
   executeMutationsAlmostAllQuery(database);
   auto end2 = std::chrono::high_resolution_clock::now();
   auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
   SPDLOG_INFO("Finished executeMutationsAlmostAllQuery in {}.{:03} seconds", duration2 / 1000, duration2 % 1000);
}

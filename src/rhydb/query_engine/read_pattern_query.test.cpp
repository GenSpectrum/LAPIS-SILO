// Validates the saneql query the WASM demo's "Reads around this position" panel
// sends: project each window position with `.at()`, then group into distinct base
// patterns with a count. This exercises the exact end-to-end path (map + at +
// groupBy + count) the browser renderer depends on, including that an uncovered
// position reconstructs to the missing symbol 'N'.
#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "rhydb/config/runtime_config.h"
#include "rhydb/database.h"
#include "rhydb/query_engine/exec_node/ndjson_sink.h"
#include "rhydb/query_engine/planner.h"
#include "rhydb/query_engine/query_plan.h"
#include "rhydb/schema/database_schema.h"

using rhydb::Database;
using rhydb::schema::TableName;

namespace {

std::vector<nlohmann::json> runQueryRows(Database& database, const std::string& query) {
   auto plan = rhydb::query_engine::Planner::planSaneqlQuery(
      query, database.tables, rhydb::config::QueryOptions{}, "read-pattern-test"
   );
   std::ostringstream out;
   rhydb::query_engine::exec_node::NdjsonSink sink{&out, plan.results_schema};
   plan.executeAndWrite(sink, 10);

   std::vector<nlohmann::json> rows;
   std::string line;
   std::istringstream stream{out.str()};
   while (std::getline(stream, line)) {
      if (!line.empty()) {
         rows.push_back(nlohmann::json::parse(line));
      }
   }
   return rows;
}

}  // namespace

TEST(ReadPatternQuery, groupsReadsIntoBasePatternsAroundPosition) {
   Database database;
   // Reference of 8 'A's; four aligned sequences (FASTA-style, offset 0):
   //   s1,s2 = ACGAAAAA   s3 = ACTAAAAA   s4 = AC (covers only positions 1-2)
   database.createNucleotideSequenceTable("default", "id", "seq", "AAAAAAAA");
   std::stringstream fasta{">s1\nACGAAAAA\n>s2\nACGAAAAA\n>s3\nACTAAAAA\n>s4\nAC\n"};
   database.appendFastaData(TableName{"default"}, fasta);

   // The window the demo builds around position 3: positions 2,3,4 -> p1,p2,p3.
   const auto rows = runQueryRows(
      database,
      "default.map({p1 := seq.at(2), p2 := seq.at(3), p3 := seq.at(4)})"
      ".groupBy({n := count()}, {p1, p2, p3})"
   );

   // Collapse to "p1p2p3" -> count for order-independent assertions.
   std::map<std::string, int> patterns;
   for (const auto& row : rows) {
      patterns
         [row["p1"].get<std::string>() + row["p2"].get<std::string>() +
          row["p3"].get<std::string>()] = row["n"].get<int>();
   }

   EXPECT_EQ(patterns["CGA"], 2);  // s1, s2: at(2)=C, at(3)=G, at(4)=A
   EXPECT_EQ(patterns["CTA"], 1);  // s3:     at(3)=T
   EXPECT_EQ(patterns["CNN"], 1);  // s4: positions 3,4 uncovered -> 'N'
   EXPECT_EQ(patterns.size(), 3U);
}

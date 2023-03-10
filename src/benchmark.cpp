#include "silo/benchmark.h"
#include <tbb/parallel_for_each.h>
#include <random>
#include <silo/common/PerfEvent.hpp>
#include "silo/query_engine/query_engine.h"

[[maybe_unused]] void silo::benchmark(
   const Database& database,
   std::istream& query_file,
   const std::string& query_directory
) {
   std::string const count_query_out_dir_str = query_directory + "count/";
   std::string const list_query_out_dir_str = query_directory + "list/";
   std::string const mutations_query_out_dir_str = query_directory + "mutations/";

   std::ofstream count_perf_table(count_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "Count-Perf " << (count_query_out_dir_str + "perf.tsv")
                << " table could not be created." << std::endl;
   }

   std::ofstream list_perf_table(list_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "List-Perf " << (list_query_out_dir_str + "perf.tsv")
                << " table could not be created." << std::endl;
   }

   std::ofstream mutations_perf_table(mutations_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "Mutations-Perf " << (mutations_query_out_dir_str + "perf.tsv")
                << " table could not be created." << std::endl;
   }

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   count_perf_table << "test_name\tparse_time\tfilter_time\taction_time\n";
   list_perf_table << "test_name\tparse_time\tfilter_time\taction_time\n";
   mutations_perf_table << "test_name\tparse_time\tfilter_time\taction_time\n";

   while (!query_file.eof() && query_file.good()) {
      std::string test_name;
      query_file >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream const test_query_file(query_directory + test_name);
      if (!test_query_file || !test_query_file.good()) {
         std::cerr << "query_file " << (query_directory + test_name) << " not found." << std::endl;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << test_query_file.rdbuf();

      unsigned const reps = 10;

      // COUNT
      int64_t parse = 0;
      int64_t filter = 0;
      int64_t action = 0;
      for (unsigned i = 0; i < reps; ++i) {
         std::string const query =
            "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/
            "},\"filter\": " +
            buffer.str() + "}";
         std::ofstream parse_file(count_query_out_dir_str + test_name + ".parse");
         std::ofstream performance_file(count_query_out_dir_str + test_name + ".perf");
         auto result = executeQuery(database, query, parse_file, performance_file);
         parse += result.parse_time;
         filter += result.filter_time;
         action += result.action_time;
      }
      count_perf_table << test_name << "\t" << parse << "\t" << filter << "\t" << action
                       << std::endl;

      // LIST
      parse = 0;
      filter = 0;
      action = 0;
      for (unsigned i = 0; i < reps; ++i) {
         std::string const query = R"({"action": {"type": "List"},"filter": )" + buffer.str() + "}";
         std::ofstream performance_file(list_query_out_dir_str + test_name + ".perf");
         auto result = executeQuery(database, query, nullstream, performance_file);
         // std::cout << result.return_message << std::endl;
         parse += result.parse_time;
         filter += result.filter_time;
         action += result.action_time;
      }
      list_perf_table << test_name << "\t" << parse << "\t" << filter << "\t" << action
                      << std::endl;

      // MUTATIONS
      parse = 0;
      filter = 0;
      action = 0;
      for (unsigned i = 0; i < reps; ++i) {
         std::string const query =
            R"({"action": {"type": "Mutations"},"filter": )" + buffer.str() + "}";
         std::ofstream performance_file(mutations_query_out_dir_str + test_name + ".perf");
         auto result = executeQuery(database, query, nullstream, performance_file);
         // std::cout << result.return_message << std::endl;
         parse += result.parse_time;
         filter += result.filter_time;
         action += result.action_time;
      }
      mutations_perf_table << test_name << "\t" << parse << "\t" << filter << "\t" << action
                           << std::endl;
   }
}

[[maybe_unused]] void silo::benchmarkThroughputMix(
   const Database& database,
   std::istream& query_file,
   const std::string& query_directory
) {
   std::string const par_query_out_dir_str = query_directory + "par/";
   std::string const ser_query_out_dir_str = query_directory + "ser/";

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   struct QueryTest {
      std::string query;
      std::string test_name;
      std::ofstream result_file;
      std::ofstream performance_file;
   };
   std::vector<QueryTest> all_queries;

   while (!query_file.eof() && query_file.good()) {
      std::string test_name;
      query_file >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream const test_query_file(query_directory + test_name);
      if (!test_query_file || !test_query_file.good()) {
         std::cerr << "query_file " << (query_directory + test_name) << " not found." << std::endl;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << test_query_file.rdbuf();
      {
         std::string const query =
            "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/
            "},\"filter\": " +
            buffer.str() + "}";
         static constexpr int COUNT_TESTS = 99;
         for (unsigned i = 0; i < COUNT_TESTS; ++i) {
            all_queries.emplace_back(QueryTest{query, test_name + "cnt"});
         }
      }

      {
         std::string const query =
            R"({"action": {"type": "Mutations"},"filter": )" + buffer.str() + "}";
         all_queries.emplace_back(QueryTest{query, test_name + "mut"});
      }
   }

   std::shuffle(all_queries.begin(), all_queries.end(), std::random_device());

   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      for (auto& query : all_queries) {
         auto result = executeQuery(database, query.query, nullstream, nullstream);
      }
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries serial." << std::endl;

   microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      tbb::parallel_for(tbb::blocked_range<size_t>(0, all_queries.size()), [&](const auto& local) {
         for (unsigned i = local.begin(); i != local.end(); i++) {
            auto& query = all_queries[i];
            auto result = executeQuery(database, query.query, nullstream, nullstream);
         }
      });
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries parallel." << std::endl;
}

[[maybe_unused]] void silo::benchmarkThroughput(
   const Database& database,
   std::istream& query_file,
   const std::string& query_directory
) {
   std::string const par_query_out_dir_str = query_directory + "par/";
   std::string const ser_query_out_dir_str = query_directory + "ser/";

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   struct query_test {
      std::string query;
      std::string test_name;
      std::ofstream result_file;
      std::ofstream performance_file;
   };
   std::vector<query_test> all_queries;

   while (!query_file.eof() && query_file.good()) {
      std::string test_name;
      query_file >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream const test_query_file(query_directory + test_name);
      if (!test_query_file || !test_query_file.good()) {
         std::cerr << "query_file " << (query_directory + test_name) << " not found." << std::endl;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << test_query_file.rdbuf();
      {
         std::string const query =
            "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/
            "},\"filter\": " +
            buffer.str() + "}";
         all_queries.emplace_back(query_test{query, test_name + "cnt"});
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      for (auto& query : all_queries) {
         auto result = executeQuery(database, query.query, nullstream, nullstream);
      }
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries serial." << std::endl;

   microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      tbb::parallel_for(tbb::blocked_range<size_t>(0, all_queries.size()), [&](const auto& local) {
         for (unsigned i = local.begin(); i != local.end(); i++) {
            auto& query = all_queries[i];
            auto result = executeQuery(database, query.query, nullstream, nullstream);
         }
      });
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries parallel." << std::endl;
}

[[maybe_unused]] void silo::benchmarkThroughputMut(
   const Database& database,
   std::istream& query_file,
   const std::string& query_directory
) {
   std::string const par_query_out_dir_str = query_directory + "par/";
   std::string const ser_query_out_dir_str = query_directory + "ser/";

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   struct query_test {
      std::string query;
      std::string test_name;
      std::ofstream result_file;
      std::ofstream performance_file;
   };
   std::vector<query_test> all_queries;

   while (!query_file.eof() && query_file.good()) {
      std::string test_name;
      query_file >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream const test_query_file(query_directory + test_name);
      if (!test_query_file || !test_query_file.good()) {
         std::cerr << "query_file " << (query_directory + test_name) << " not found." << std::endl;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << test_query_file.rdbuf();
      {
         std::string const query =
            R"({"action": {"type": "Mutations"},"filter": )" + buffer.str() + "}";
         all_queries.emplace_back(query_test{query, test_name + "mut"});
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      for (auto& query : all_queries) {
         auto result = executeQuery(database, query.query, nullstream, nullstream);
      }
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries serial." << std::endl;

   microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      tbb::parallel_for(tbb::blocked_range<size_t>(0, all_queries.size()), [&](const auto& local) {
         for (unsigned i = local.begin(); i != local.end(); i++) {
            auto& query = all_queries[i];
            auto result = executeQuery(database, query.query, nullstream, nullstream);
         }
      });
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries parallel." << std::endl;
}

#include "silo/benchmark.h"
#include <tbb/parallel_for_each.h>
#include <random>
#include <silo/common/PerfEvent.hpp>
#include "silo/query_engine/query_engine.h"

using namespace silo;

int silo::benchmark(
   const Database& db,
   std::istream& query_defs,
   const std::string& query_dir_str
) {
   std::string count_query_out_dir_str = query_dir_str + "count/";
   std::string list_query_out_dir_str = query_dir_str + "list/";
   std::string mutations_query_out_dir_str = query_dir_str + "mutations/";

   std::ofstream count_perf_table(count_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "Count-Perf " << (count_query_out_dir_str + "perf.tsv")
                << " table could not be created." << std::endl;
      return 0;
   }

   std::ofstream list_perf_table(list_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "List-Perf " << (list_query_out_dir_str + "perf.tsv")
                << " table could not be created." << std::endl;
      return 0;
   }

   std::ofstream mutations_perf_table(mutations_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "Mutations-Perf " << (mutations_query_out_dir_str + "perf.tsv")
                << " table could not be created." << std::endl;
      return 0;
   }

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   count_perf_table << "test_name\tparse_time\tfilter_time\taction_time\n";
   list_perf_table << "test_name\tparse_time\tfilter_time\taction_time\n";
   mutations_perf_table << "test_name\tparse_time\tfilter_time\taction_time\n";

   while (!query_defs.eof() && query_defs.good()) {
      std::string test_name;
      query_defs >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream query_file(query_dir_str + test_name);
      if (!query_file || !query_file.good()) {
         std::cerr << "query_file " << (query_dir_str + test_name) << " not found." << std::endl;
         return 0;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << query_file.rdbuf();

      unsigned reps = 10;

      // COUNT
      int64_t parse = 0;
      int64_t filter = 0;
      int64_t action = 0;
      for (unsigned i = 0; i < reps; ++i) {
         std::string query =
            "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/
            "},\"filter\": " +
            buffer.str() + "}";
         std::ofstream parse_file(count_query_out_dir_str + test_name + ".parse");
         std::ofstream performance_file(count_query_out_dir_str + test_name + ".perf");
         auto result = execute_query(db, query, parse_file, performance_file);
         // std::cout << result.return_message << std::endl;
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
         std::string query = "{\"action\": {\"type\": \"List\"},\"filter\": " + buffer.str() + "}";
         std::ofstream performance_file(list_query_out_dir_str + test_name + ".perf");
         auto result = execute_query(db, query, nullstream, performance_file);
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
         std::string query =
            "{\"action\": {\"type\": \"Mutations\"},\"filter\": " + buffer.str() + "}";
         std::ofstream performance_file(mutations_query_out_dir_str + test_name + ".perf");
         auto result = execute_query(db, query, nullstream, performance_file);
         // std::cout << result.return_message << std::endl;
         parse += result.parse_time;
         filter += result.filter_time;
         action += result.action_time;
      }
      mutations_perf_table << test_name << "\t" << parse << "\t" << filter << "\t" << action
                           << std::endl;
   }
   return 0;
}

int silo::benchmark_throughput_mix(
   const Database& db,
   std::istream& query_defs,
   const std::string& query_dir_str
) {
   std::string par_query_out_dir_str = query_dir_str + "par/";
   std::string ser_query_out_dir_str = query_dir_str + "ser/";

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   struct query_test {
      std::string query;
      std::string test_name;
      std::ofstream result_file;
      std::ofstream performance_file;
   };
   std::vector<query_test> all_queries;

   while (!query_defs.eof() && query_defs.good()) {
      std::string test_name;
      query_defs >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream query_file(query_dir_str + test_name);
      if (!query_file || !query_file.good()) {
         std::cerr << "query_file " << (query_dir_str + test_name) << " not found." << std::endl;
         return 0;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << query_file.rdbuf();
      {
         std::string query =
            "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/
            "},\"filter\": " +
            buffer.str() + "}";
         for (unsigned i = 0; i < 99; ++i)
            all_queries.emplace_back(query_test{query, test_name + "cnt"});
      }

      {
         std::string query =
            "{\"action\": {\"type\": \"Mutations\"},\"filter\": " + buffer.str() + "}";
         all_queries.emplace_back(query_test{query, test_name + "mut"});
      }
   }

   std::shuffle(all_queries.begin(), all_queries.end(), std::random_device());

   int64_t microseconds = 0;
   {
      BlockTimer timer(microseconds);
      for (auto& query : all_queries) {
         auto result = execute_query(db, query.query, nullstream, nullstream);
      }
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries serial." << std::endl;

   microseconds = 0;
   {
      BlockTimer timer(microseconds);
      tbb::parallel_for(tbb::blocked_range<size_t>(0, all_queries.size()), [&](const auto& local) {
         for (unsigned i = local.begin(); i != local.end(); i++) {
            auto& query = all_queries[i];
            auto result = execute_query(db, query.query, nullstream, nullstream);
         }
      });
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries parallel." << std::endl;

   return 0;
}

int silo::benchmark_throughput(
   const Database& db,
   std::istream& query_defs,
   const std::string& query_dir_str
) {
   std::string par_query_out_dir_str = query_dir_str + "par/";
   std::string ser_query_out_dir_str = query_dir_str + "ser/";

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   struct query_test {
      std::string query;
      std::string test_name;
      std::ofstream result_file;
      std::ofstream performance_file;
   };
   std::vector<query_test> all_queries;

   while (!query_defs.eof() && query_defs.good()) {
      std::string test_name;
      query_defs >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream query_file(query_dir_str + test_name);
      if (!query_file || !query_file.good()) {
         std::cerr << "query_file " << (query_dir_str + test_name) << " not found." << std::endl;
         return 0;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << query_file.rdbuf();
      {
         std::string query =
            "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/
            "},\"filter\": " +
            buffer.str() + "}";
         all_queries.emplace_back(query_test{query, test_name + "cnt"});
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer timer(microseconds);
      for (auto& query : all_queries) {
         auto result = execute_query(db, query.query, nullstream, nullstream);
      }
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries serial." << std::endl;

   microseconds = 0;
   {
      BlockTimer timer(microseconds);
      tbb::parallel_for(tbb::blocked_range<size_t>(0, all_queries.size()), [&](const auto& local) {
         for (unsigned i = local.begin(); i != local.end(); i++) {
            auto& query = all_queries[i];
            auto result = execute_query(db, query.query, nullstream, nullstream);
         }
      });
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries parallel." << std::endl;

   return 0;
}

int silo::benchmark_throughput_mut(
   const Database& db,
   std::istream& query_defs,
   const std::string& query_dir_str
) {
   std::string par_query_out_dir_str = query_dir_str + "par/";
   std::string ser_query_out_dir_str = query_dir_str + "ser/";

   std::ofstream nullstream("/dev/null", std::ofstream::out | std::ofstream::app);

   struct query_test {
      std::string query;
      std::string test_name;
      std::ofstream result_file;
      std::ofstream performance_file;
   };
   std::vector<query_test> all_queries;

   while (!query_defs.eof() && query_defs.good()) {
      std::string test_name;
      query_defs >> test_name;
      if (test_name.empty()) {
         break;
      }
      std::ifstream query_file(query_dir_str + test_name);
      if (!query_file || !query_file.good()) {
         std::cerr << "query_file " << (query_dir_str + test_name) << " not found." << std::endl;
         return 0;
      }

      std::cerr << "query: " << test_name << std::endl;
      std::stringstream buffer;
      buffer << query_file.rdbuf();
      {
         std::string query =
            "{\"action\": {\"type\": \"Mutations\"},\"filter\": " + buffer.str() + "}";
         all_queries.emplace_back(query_test{query, test_name + "mut"});
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer timer(microseconds);
      for (auto& query : all_queries) {
         auto result = execute_query(db, query.query, nullstream, nullstream);
      }
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries serial." << std::endl;

   microseconds = 0;
   {
      BlockTimer timer(microseconds);
      tbb::parallel_for(tbb::blocked_range<size_t>(0, all_queries.size()), [&](const auto& local) {
         for (unsigned i = local.begin(); i != local.end(); i++) {
            auto& query = all_queries[i];
            auto result = execute_query(db, query.query, nullstream, nullstream);
         }
      });
   }
   std::cout << "Took " << microseconds << " microseconds for " << all_queries.size()
             << " queries parallel." << std::endl;

   return 0;
}

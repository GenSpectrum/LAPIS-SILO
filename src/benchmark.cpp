//
// Created by Alexander Taepper on 28.09.22.
//

#include "silo/benchmark.h"
#include "silo/query_engine.h"

using namespace silo;

int silo::benchmark(const Database& db, std::istream& query_defs, const std::string& query_dir_str) {
   std::string count_query_out_dir_str = query_dir_str + "count/";
   std::string list_query_out_dir_str = query_dir_str + "list/";
   std::string mutations_query_out_dir_str = query_dir_str + "mutations/";

   std::ofstream count_perf_table(count_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "Count-Perf " << (count_query_out_dir_str + "perf.tsv") << " table could not be created." << std::endl;
      return 0;
   }

   std::ofstream list_perf_table(list_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "List-Perf " << (list_query_out_dir_str + "perf.tsv") << " table could not be created." << std::endl;
      return 0;
   }

   std::ofstream mutations_perf_table(mutations_query_out_dir_str + "perf.tsv");
   if (!count_perf_table) {
      std::cerr << "Mutations-Perf " << (mutations_query_out_dir_str + "perf.tsv") << " table could not be created." << std::endl;
      return 0;
   }

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

      // COUNT
      {
         std::string query = "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/ "},\"filter\": " + buffer.str() + "}";
         std::ofstream result_file(count_query_out_dir_str + test_name + ".res");
         std::ofstream performance_file(count_query_out_dir_str + test_name + ".perf");
         auto result = execute_query(db, query, result_file, performance_file);
         std::cout << result.return_message << std::endl;
         count_perf_table << test_name << "\t" << result.parse_time << "\t" << result.filter_time << "\t" << result.action_time << std::endl;
      }

      // LIST
      {
         std::string query = "{\"action\": {\"type\": \"List\"},\"filter\": " + buffer.str() + "}";
         std::ofstream result_file(list_query_out_dir_str + test_name + ".res");
         std::ofstream performance_file(list_query_out_dir_str + test_name + ".perf");
         auto result = execute_query(db, query, result_file, performance_file);
         std::cout << result.return_message << std::endl;
         list_perf_table << test_name << "\t" << result.parse_time << "\t" << result.filter_time << "\t" << result.action_time << std::endl;
      }

      // MUTATIONS
      {
         std::string query = "{\"action\": {\"type\": \"Mutations\"},\"filter\": " + buffer.str() + "}";
         std::ofstream result_file(mutations_query_out_dir_str + test_name + ".res");
         std::ofstream performance_file(mutations_query_out_dir_str + test_name + ".perf");
         auto result = execute_query(db, query, result_file, performance_file);
         std::cout << result.return_message << std::endl;
         mutations_perf_table << test_name << "\t" << result.parse_time << "\t" << result.filter_time << "\t" << result.action_time << std::endl;
      }
   }
   return 0;
}

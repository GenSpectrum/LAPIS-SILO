#ifndef SILO_BENCHMARK_H
#define SILO_BENCHMARK_H

#include "silo/database.h"

namespace silo {
// TODO(someone): remove these benchmark tests
[[maybe_unused]] void benchmark(
   const silo::Database& database,
   std::istream& query_file,
   const std::string& query_directory
);
[[maybe_unused]] void benchmarkThroughput(
   const silo::Database& database,
   std::istream& query_file,
   const std::string& query_directory
);
[[maybe_unused]] void benchmarkThroughputMix(
   const silo::Database& database,
   std::istream& query_file,
   const std::string& query_directory
);
[[maybe_unused]] void benchmarkThroughputMut(
   const silo::Database& database,
   std::istream& query_file,
   const std::string& query_directory
);

}  // namespace silo

#endif  // SILO_BENCHMARK_H

//
// Created by Alexander Taepper on 05.01.23.
//

#ifndef SILO_BENCHMARK_H
#define SILO_BENCHMARK_H

#include "silo/database.h"

namespace silo {

int benchmark(const silo::Database& db, std::istream& query_defs, const std::string& query_dir_str);
int benchmark_throughput(const silo::Database& db, std::istream& query_defs, const std::string& query_dir_str);
int benchmark_throughput_mix(const silo::Database& db, std::istream& query_defs, const std::string& query_dir_str);
int benchmark_throughput_mut(const silo::Database& db, std::istream& query_defs, const std::string& query_dir_str);

}

#endif //SILO_BENCHMARK_H

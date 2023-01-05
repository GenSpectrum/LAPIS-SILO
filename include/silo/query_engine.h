//
// Created by Alexander Taepper on 27.09.22.
//

#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include "silo/database.h"
#include <string>

namespace silo {

struct result_s {
   std::string return_message;
   int64_t parse_time;
   int64_t execution_time;
};

result_s execute_query(const Database& db, const std::string& query, std::ostream& res_out, std::ostream& perf_out);

result_s analyse_query(const Database& db, const std::string& query);

} // namespace silo;

#endif //SILO_QUERY_ENGINE_H

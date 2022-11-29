//
// Created by Alexander Taepper on 27.09.22.
//

#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H
#include <string>
#include "silo/database.h"

namespace silo {

std::string execute_query(const Database& db, const std::istream& query);

} // namespace silo;

#endif //SILO_QUERY_ENGINE_H

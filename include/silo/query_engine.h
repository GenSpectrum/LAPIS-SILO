//
// Created by Alexander Taepper on 27.09.22.
//

#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include "sequence_store.h"
#include "rapidjson/document.h"

namespace silo {

   std::string execute_query(const std::string& query);

}

#endif //SILO_QUERY_ENGINE_H

//
// Created by Alexander Taepper on 16.11.22.
//

#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <silo/meta_store.h>
#include <silo/query_engine.h>
#include <silo/sequence_store.h>
#include <silo/silo.h>

namespace silo {

struct DatabasePartition {
   MetaStore mdb;
   SequenceStore db;
};

struct Database {
   std::vector<DatabasePartition> partitions;
};

} // namespace silo

#endif //SILO_DATABASE_H

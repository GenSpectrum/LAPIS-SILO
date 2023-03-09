//
// Created by Alexander Taepper on 02.02.23.
//

#ifndef SILO_BOOTSTRAP_H
#define SILO_BOOTSTRAP_H

#include <silo/database.h>

int bootstrap(const silo::Database& db, std::string& out_dir, unsigned seed, unsigned factor);

#endif  // SILO_BOOTSTRAP_H

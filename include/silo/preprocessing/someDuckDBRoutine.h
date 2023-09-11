#pragma once

#include <string_view>

#include "silo/database.h"

namespace silo {

void executeDuckDBRoutine(
   const silo::Database& database,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name
);

}
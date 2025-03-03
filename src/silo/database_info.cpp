#include "silo/database_info.h"

using silo::DatabaseInfo;

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void silo::to_json(nlohmann::json& json, const DatabaseInfo& databaseInfo) {
   json = nlohmann::json{
      {"version", databaseInfo.version},
      {"sequenceCount", databaseInfo.sequence_count},
      {"totalSize", databaseInfo.total_size},
      {"nBitmapsSize", databaseInfo.n_bitmaps_size},
      {"numberOfPartitions", databaseInfo.number_of_partitions}
   };
}
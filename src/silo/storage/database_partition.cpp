#include "silo/storage/database_partition.h"

#include "silo/storage/dictionary.h"

namespace silo {

const std::vector<preprocessing::Chunk>& DatabasePartition::getChunks() const {
   return chunks;
}

}  // namespace silo

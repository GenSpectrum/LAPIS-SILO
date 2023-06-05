#ifndef SILO_DATABASE_PARTITION_H
#define SILO_DATABASE_PARTITION_H

#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "silo/preprocessing/partition.h"
#include "silo/storage/metadata_store.h"
#include "silo/storage/sequence_store.h"

namespace silo {

class DatabasePartition {
   friend class Database;
   friend class boost::serialization::
      access;  // here because serialize is private member
               // (https://www.boost.org/doc/libs/1_34_0/libs/serialization/doc/serialization.html)

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& meta_store;
      archive& seq_store;
      archive& sequenceCount;
      archive& chunks;
   }

   std::vector<silo::preprocessing::Chunk> chunks;

  public:
   MetadataStore meta_store;
   SequenceStore seq_store;
   unsigned sequenceCount;

   [[nodiscard]] const std::vector<silo::preprocessing::Chunk>& getChunks() const;
};

}  // namespace silo

#endif  // SILO_DATABASE_PARTITION_H

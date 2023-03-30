#ifndef SILO_DATABASE_PARTITION_H
#define SILO_DATABASE_PARTITION_H

#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "silo/storage/metadata_store.h"
#include "silo/storage/sequence_store.h"

class Dictionary;

namespace silo {

struct Chunk {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& prefix;
      archive& count;
      archive& offset;
      archive& pango_lineages;
   }
   std::string prefix;
   uint32_t count;
   uint32_t offset;
   std::vector<std::string> pango_lineages;
};

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

   std::vector<silo::Chunk> chunks;

  public:
   MetadataStore meta_store;
   SequenceStore seq_store;
   unsigned sequenceCount;

   [[nodiscard]] const std::vector<silo::Chunk>& getChunks() const;

   void finalizeBuild(const Dictionary& dict);
};

}  // namespace silo

#endif  // SILO_DATABASE_PARTITION_H

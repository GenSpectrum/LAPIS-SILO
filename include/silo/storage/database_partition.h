#ifndef SILO_DATABASE_PARTITION_H
#define SILO_DATABASE_PARTITION_H

#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "silo/preprocessing/partition.h"
#include "silo/storage/column_group.h"
#include "silo/storage/sequence_store.h"

namespace silo {

class DatabasePartition {
   friend class boost::serialization::
      access;  // here because serialize is private member
               // (https://www.boost.org/doc/libs/1_34_0/libs/serialization/doc/serialization.html)

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      // clang-format off
      archive& columns;
      archive& seq_store;
      archive& sequenceCount;
      archive& chunks;
      // clang-format on
   }

  public:
   std::vector<silo::preprocessing::Chunk> chunks;
   storage::ColumnGroup columns;
   SequenceStore seq_store;
   unsigned sequenceCount;

   [[nodiscard]] const std::vector<preprocessing::Chunk>& getChunks() const;

   void insertColumn(const std::string& name, storage::column::StringColumnPartition& column);
   void insertColumn(
      const std::string& name,
      storage::column::IndexedStringColumnPartition& column
   );
   void insertColumn(const std::string& name, storage::column::IntColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::DateColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::PangoLineageColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::FloatColumnPartition& column);
};

}  // namespace silo

#endif  // SILO_DATABASE_PARTITION_H

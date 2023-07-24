#ifndef SILO_DATABASE_PARTITION_H
#define SILO_DATABASE_PARTITION_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>

#include "silo/preprocessing/partition.h"
#include "silo/storage/aa_store.h"
#include "silo/storage/column_group.h"
#include "silo/storage/sequence_store.h"

namespace boost {
namespace serialization {
class access;
}  // namespace serialization
}  // namespace boost

namespace silo {
class AAStorePartition;
class SequenceStorePartition;
namespace storage::column {
class DateColumnPartition;
class FloatColumnPartition;
class IndexedStringColumnPartition;
class IntColumnPartition;
class PangoLineageColumnPartition;
class StringColumnPartition;
}  // namespace storage::column

class DatabasePartition {
   friend class boost::serialization::
      access;  // here because serialize is private member
               // (https://www.boost.org/doc/libs/1_34_0/libs/serialization/doc/serialization.html)

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & chunks;
      // clang-format on
   }

  public:
   template <class Archive>
   /// The data of partitions is serialized in parallel. Therefore it is not part of the default
   /// serialization method
   void serializeData(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & columns;
      for(auto& [name, store] : nuc_sequences){
         archive & store;
      }
      for(auto& [name, store] : aa_sequences){
         archive & store;
      }
      archive & sequenceCount;
      // clang-format on
   }

  private:
   std::vector<silo::preprocessing::Chunk> chunks;

   DatabasePartition() = default;

  public:
   explicit DatabasePartition(std::vector<silo::preprocessing::Chunk> chunks);

   storage::ColumnPartitionGroup columns;
   std::map<std::string, SequenceStorePartition&> nuc_sequences;
   std::map<std::string, AAStorePartition&> aa_sequences;
   uint32_t sequenceCount;

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

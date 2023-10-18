#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "silo/preprocessing/partition.h"
#include "silo/storage/column_group.h"

namespace boost {
namespace serialization {
class access;
}  // namespace serialization
}  // namespace boost

namespace silo {
class AminoAcid;
class Nucleotide;
namespace storage {
namespace column {
class DateColumnPartition;
class FloatColumnPartition;
class IndexedStringColumnPartition;
class IntColumnPartition;
class PangoLineageColumnPartition;
class StringColumnPartition;
template <typename SymbolType>
class InsertionColumnPartition;
}  // namespace column
}  // namespace storage
template <typename SymbolType>
class SequenceStorePartition;
}  // namespace silo

namespace silo {

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
      archive & sequence_count;
      // clang-format on
   }

  private:
   std::vector<silo::preprocessing::PartitionChunk> chunks;

   DatabasePartition() = default;

  public:
   storage::ColumnPartitionGroup columns;
   std::map<std::string, SequenceStorePartition<Nucleotide>&> nuc_sequences;
   std::map<std::string, SequenceStorePartition<AminoAcid>&> aa_sequences;
   uint32_t sequence_count = 0;

   explicit DatabasePartition(std::vector<silo::preprocessing::PartitionChunk> chunks);

   void flipBitmaps();

   [[nodiscard]] const std::vector<preprocessing::PartitionChunk>& getChunks() const;

   void insertColumn(const std::string& name, storage::column::StringColumnPartition& column);
   void insertColumn(
      const std::string& name,
      storage::column::IndexedStringColumnPartition& column
   );
   void insertColumn(const std::string& name, storage::column::IntColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::DateColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::PangoLineageColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::FloatColumnPartition& column);
   void insertColumn(
      const std::string& name,
      storage::column::InsertionColumnPartition<Nucleotide>& column
   );
   void insertColumn(
      const std::string& name,
      storage::column::InsertionColumnPartition<AminoAcid>& column
   );

   template <typename SymbolType>
   const std::map<std::string, SequenceStorePartition<SymbolType>&>& getSequenceStores() const;
};

}  // namespace silo

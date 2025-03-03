#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/sequence_store.h"
#include "silo/storage/unaligned_sequence_store.h"

namespace silo {

class DatabasePartition {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
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

   storage::ColumnPartitionGroup columns;
   std::map<std::string, SequenceStorePartition<Nucleotide>&> nuc_sequences;
   std::map<std::string, UnalignedSequenceStorePartition&> unaligned_nuc_sequences;
   std::map<std::string, SequenceStorePartition<AminoAcid>&> aa_sequences;
   uint32_t sequence_count = 0;

  private:
   void validateNucleotideSequences() const;

   void validateAminoAcidSequences() const;

   void validateMetadataColumns() const;

   template <typename ColumnPartition>
   void validateColumnsHaveSize(
      const std::map<std::string, ColumnPartition>& columnsOfTheType,
      const std::string& columnType
   ) const;

  public:
   DatabasePartition() = default;

   void validate() const;

   void insertColumn(const std::string& name, storage::column::StringColumnPartition& column);
   void insertColumn(
      const std::string& name,
      storage::column::IndexedStringColumnPartition& column
   );
   void insertColumn(const std::string& name, storage::column::BoolColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::IntColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::DateColumnPartition& column);
   void insertColumn(const std::string& name, storage::column::FloatColumnPartition& column);

   template <typename SymbolType>
   const std::map<std::string, SequenceStorePartition<SymbolType>&>& getSequenceStores() const;
};

}  // namespace silo

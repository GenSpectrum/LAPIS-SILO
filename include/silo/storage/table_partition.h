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
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/unaligned_sequence_store.h"
#include "silo/storage/column_group.h"

namespace silo::storage {

class TablePartition {
  public:
   template <class Archive>
   /// The data of partitions is serialized in parallel. Therefore it is not part of the default
   /// serialization method
   void serializeData(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & columns;
      archive & sequence_count;
      // clang-format on
   }

   storage::ColumnPartitionGroup columns;
   uint32_t sequence_count = 0;

   explicit TablePartition(schema::TableSchema& schema);

   void validate() const;

   void finalize();

  private:
   void validateNucleotideSequences() const;

   void validateAminoAcidSequences() const;

   void validateMetadataColumns() const;

   template <typename ColumnPartition>
   void validateColumnsHaveSize(
      const std::map<std::string, ColumnPartition>& columnsOfTheType,
      const std::string& columnType
   ) const;
};

}  // namespace silo::storage

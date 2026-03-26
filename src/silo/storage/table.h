#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>

#include <boost/serialization/access.hpp>

#include "silo/schema/database_schema.h"
#include "silo/storage/column_group.h"

namespace silo::storage {

class Table {
  public:
   std::shared_ptr<schema::TableSchema> schema;
   storage::ColumnPartitionGroup columns;
   uint32_t sequence_count = 0;

   explicit Table(std::shared_ptr<schema::TableSchema> schema);

   Table(Table&& other) = default;
   Table& operator=(Table&& other) = default;

   Table(const Table& other) = delete;
   Table& operator=(const Table& other) = delete;

   template <class Archive>
   void serializeData(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & columns;
      archive & sequence_count;
      // clang-format on
   }

   void validate() const;

   void finalize();

   void loadData(const std::filesystem::path& path);
   void saveData(const std::filesystem::path& path);
   void validatePrimaryKeyUnique() const;

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

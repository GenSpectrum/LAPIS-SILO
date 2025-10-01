#include "silo/storage/table.h"

#include <fstream>
#include <unordered_set>

#include <spdlog/spdlog.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/detail/interface_iarchive.hpp>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "evobench/evobench.hpp"
#include "silo/persistence/exception.h"
#include "silo/roaring_util/roaring_serialize.h"
#include "silo/schema/duplicate_primary_key_exception.h"

namespace silo::storage {

size_t Table::getNumberOfPartitions() const {
   return partitions.size();
}

void Table::validate() const {
   validatePrimaryKeyUnique();
}

void Table::validatePrimaryKeyUnique() const {
   SPDLOG_DEBUG("Checking that primary keys are unique.");
   const auto primary_key = schema.primary_key;
   SILO_ASSERT(primary_key.type == schema::ColumnType::STRING);

   size_t total_rows = 0;
   for (const auto& partition : partitions) {
      total_rows += partition->sequence_count;
   }

   std::unordered_set<std::string> unique_keys;
   unique_keys.reserve(total_rows);
   for (const auto& partition : partitions) {
      auto& primary_key_column = partition->columns.string_columns.at(primary_key.name);
      auto num_values = primary_key_column.numValues();
      for (size_t i = 0; i < num_values; ++i) {
         std::string value = primary_key_column.getValueString(i);
         if (unique_keys.contains(value)) {
            throw schema::DuplicatePrimaryKeyException("Found duplicate primary key {}", value);
         }
         unique_keys.insert(value);
      }
   }
   SPDLOG_DEBUG("Found {} distinct primary keys.", unique_keys.size());
}

std::shared_ptr<TablePartition> Table::getPartition(size_t partition_idx) const {
   return partitions.at(partition_idx);
}

std::shared_ptr<TablePartition> Table::getPartition(size_t partition_idx) {
   return partitions.at(partition_idx);
}

std::shared_ptr<storage::TablePartition> Table::addPartition() {
   return partitions.emplace_back(std::make_shared<TablePartition>(schema));
}

namespace {

std::ifstream openInputFileOrThrow(const std::string& path) {
   std::ifstream file(path, std::ios::binary);
   if (!file) {
      auto error = fmt::format("Input file {} could not be opened.", path);
      throw persistence::LoadDatabaseException(error);
   }
   return file;
}

std::ofstream openOutputFileOrThrow(const std::string& path) {
   std::ofstream file(path, std::ios::binary);
   if (!file) {
      auto error = fmt::format("Output file {} could not be opened.", path);
      throw persistence::SaveDatabaseException(error);
   }
   return file;
}

}  // namespace

void Table::saveData(const std::filesystem::path& save_directory) {
   EVOBENCH_SCOPE("Table", "saveData");
   std::vector<std::ofstream> partition_archives;
   for (uint32_t i = 0; i < getNumberOfPartitions(); ++i) {
      const auto& partition_archive = save_directory / ("P" + std::to_string(i) + ".silo");
      partition_archives.emplace_back(openOutputFileOrThrow(partition_archive));

      if (!partition_archives.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition output file " + partition_archive.string() + " for saving"
         );
      }
   }

   SPDLOG_INFO("Saving {} partitions...", getNumberOfPartitions());
   for (size_t partition_idx = 0; partition_idx < getNumberOfPartitions(); ++partition_idx) {
      ::boost::archive::binary_oarchive output_archive(partition_archives[partition_idx]);
      partitions[partition_idx]->serializeData(output_archive, 0);
   }
   SPDLOG_INFO("Finished saving partitions");
}

void Table::loadData(const std::filesystem::path& save_directory) {
   EVOBENCH_SCOPE("Table", "loadData");
   std::vector<std::ifstream> file_vec;
   for (const std::filesystem::path& file : std::filesystem::directory_iterator(save_directory)) {
      if (file.extension() == ".silo") {
         file_vec.emplace_back(openInputFileOrThrow(file));
         addPartition();

         if (!file_vec.back()) {
            throw persistence::SaveDatabaseException(
               fmt::format("Cannot open partition input file {} for loading", file.string())
            );
         }
      }
   }
   for (size_t partition_index = 0; partition_index != getNumberOfPartitions(); ++partition_index) {
      ::boost::archive::binary_iarchive input_archive(file_vec[partition_index]);
      partitions[partition_index]->serializeData(input_archive, 0);
   }
   SPDLOG_INFO("Finished loading partition data");
}

}  // namespace silo::storage

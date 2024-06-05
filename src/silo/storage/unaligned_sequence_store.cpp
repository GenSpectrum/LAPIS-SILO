#include "silo/storage/unaligned_sequence_store.h"

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/persistence/exception.h"
#include "silo/preprocessing/preprocessing_exception.h"

silo::UnalignedSequenceStorePartition::UnalignedSequenceStorePartition(
   std::string sql_for_reading_file,
   const std::string& compression_dictionary
)
    : sql_for_reading_file(std::move(sql_for_reading_file)),
      compression_dictionary(compression_dictionary) {}

std::string silo::UnalignedSequenceStorePartition::getReadSQL() const {
   return sql_for_reading_file;
}

silo::UnalignedSequenceStore::UnalignedSequenceStore(
   std::filesystem::path folder_path,
   std::string&& compression_dictionary
)
    : folder_path(std::move(folder_path)),
      compression_dictionary(std::move(compression_dictionary)) {}

silo::UnalignedSequenceStorePartition& silo::UnalignedSequenceStore::createPartition() {
   const size_t partition_id = partitions.size();
   return partitions.emplace_back(
      fmt::format(
         "SELECT * FROM read_parquet('{}/*/*.parquet', hive_partitioning = 1) "
         "WHERE partition_id = {}",
         folder_path.string(),
         partition_id
      ),
      compression_dictionary
   );
}

void silo::UnalignedSequenceStore::saveFolder(const std::filesystem::path& save_location) const {
   std::filesystem::copy(folder_path, save_location, std::filesystem::copy_options::recursive);
}

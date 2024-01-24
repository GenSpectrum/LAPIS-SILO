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
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

silo::UnalignedSequenceStorePartition::UnalignedSequenceStorePartition(
   std::filesystem::path file_name,
   std::string& compression_dictionary
)
    : file_name(std::move(file_name)),
      compression_dictionary(compression_dictionary) {}

size_t silo::UnalignedSequenceStorePartition::fill(silo::ZstdFastaTableReader& input) {
   const size_t line_count = input.lineCount();

   input.copyTableTo(file_name.string());

   sequence_count += line_count;
   return line_count;
}

silo::UnalignedSequenceStore::UnalignedSequenceStore(
   std::filesystem::path folder_path,
   std::string&& compression_dictionary
)
    : folder_path(std::move(folder_path)),
      compression_dictionary(std::move(compression_dictionary)) {}

silo::UnalignedSequenceStorePartition& silo::UnalignedSequenceStore::createPartition() {
   return partitions.emplace_back(
      folder_path / fmt::format("P{}.parquet", partitions.size()), compression_dictionary
   );
}

void silo::UnalignedSequenceStore::saveFolder(const std::filesystem::path& save_location) const {
   std::filesystem::copy(folder_path, save_location);
}

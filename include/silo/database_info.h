#ifndef SILO_INCLUDE_SILO_API_DATABASE_INFO_H_
#define SILO_INCLUDE_SILO_API_DATABASE_INFO_H_

#include <cinttypes>
#include <map>
#include <vector>

#include "silo/common/nucleotide_symbols.h"

namespace silo {

struct DatabaseInfo {
   uint32_t sequence_count;
   uint64_t total_size;
   size_t n_bitmaps_size;
};

struct BitmapSizePerSymbol {
   std::map<Nucleotide::Symbol, uint64_t> size_in_bytes;
   BitmapSizePerSymbol& operator+=(const BitmapSizePerSymbol& other);
   BitmapSizePerSymbol();
};

struct BitmapContainerSizeStatistic {
   uint32_t number_of_array_containers;
   uint32_t number_of_run_containers;
   uint32_t number_of_bitset_containers;

   uint32_t number_of_values_stored_in_array_containers;
   uint32_t number_of_values_stored_in_run_containers;
   uint32_t number_of_values_stored_in_bitset_containers;

   uint64_t total_bitmap_size_array_containers;
   uint64_t total_bitmap_size_run_containers;
   uint64_t total_bitmap_size_bitset_containers;
};

struct BitmapContainerSize {
   size_t section_length;
   std::map<std::string, std::vector<size_t>> size_per_genome_symbol_and_section;

   BitmapContainerSizeStatistic bitmap_container_size_statistic;

   uint64_t total_bitmap_size_frozen;
   uint64_t total_bitmap_size_computed;

   explicit BitmapContainerSize(size_t genome_length, size_t section_length);

   BitmapContainerSize& operator+=(const BitmapContainerSize& other);
};

struct SequenceStoreStatistics {
   BitmapSizePerSymbol bitmap_size_per_symbol;
   BitmapContainerSize bitmap_container_size_per_genome_section;
};

struct DetailedDatabaseInfo {
   std::unordered_map<std::string, SequenceStoreStatistics> sequences;
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_API_DATABASE_INFO_H_

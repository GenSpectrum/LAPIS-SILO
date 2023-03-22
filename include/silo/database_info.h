#ifndef SILO_INCLUDE_SILO_API_DATABASE_INFO_H_
#define SILO_INCLUDE_SILO_API_DATABASE_INFO_H_

#include <cinttypes>
#include <map>
#include <vector>

namespace silo {

enum class GENOME_SYMBOL;

struct DatabaseInfo {
   uint32_t sequence_count;
   uint64_t total_size;
   size_t n_bitmaps_size;
};

struct BitmapSizePerSymbol {
   std::map<GENOME_SYMBOL, uint64_t> size_in_bytes;
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
   uint32_t section_length;
   std::map<std::string, std::vector<uint32_t>> size_per_genome_symbol_and_section;

   BitmapContainerSizeStatistic bitmap_container_size_statistic;

   uint64_t total_bitmap_size_frozen;
   uint64_t total_bitmap_size_computed;

   explicit BitmapContainerSize(uint32_t section_length);

   BitmapContainerSize& operator+=(const BitmapContainerSize& other);
};

struct DetailedDatabaseInfo {
   BitmapSizePerSymbol bitmap_size_per_symbol;
   BitmapContainerSize bitmap_container_size_per_genome_section;
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_API_DATABASE_INFO_H_

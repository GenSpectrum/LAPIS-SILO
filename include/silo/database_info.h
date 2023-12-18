#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo {

struct DatabaseInfo {
   uint32_t sequence_count;
   uint64_t total_size;
   size_t n_bitmaps_size;
};

}  // namespace silo

template <>
struct [[maybe_unused]] fmt::formatter<silo::DatabaseInfo> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(silo::DatabaseInfo database_info, format_context& ctx)
      -> decltype(ctx.out()) {
      return format_to(
         ctx.out(),
         "sequence count: {}, total size: {}, N bitmaps size: {}",
         database_info.sequence_count,
         silo::formatNumber(database_info.total_size),
         silo::formatNumber(database_info.n_bitmaps_size)
      );
   }
};

namespace silo {

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

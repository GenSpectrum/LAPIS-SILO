#ifndef SILO_INCLUDE_SILO_API_DATABASE_INFO_H_
#define SILO_INCLUDE_SILO_API_DATABASE_INFO_H_

#include <cinttypes>
#include <map>
#include <vector>

namespace silo {

enum class GENOME_SYMBOL;

struct DatabaseInfo {
   uint32_t sequenceCount;  // NOLINT(readability-identifier-naming)
   uint64_t totalSize;      // NOLINT(readability-identifier-naming)
   size_t nBitmapsSize;     // NOLINT(readability-identifier-naming)
};

struct BitmapSizePerSymbol {
   std::map<GENOME_SYMBOL, uint64_t> sizeInBytes;  // NOLINT(readability-identifier-naming)
   BitmapSizePerSymbol& operator+=(const BitmapSizePerSymbol& other);
   BitmapSizePerSymbol();
};

struct BitmapContainerSizeStatistic {
   uint32_t numberOfArrayContainers;   // NOLINT(readability-identifier-naming)
   uint32_t numberOfRunContainers;     // NOLINT(readability-identifier-naming)
   uint32_t numberOfBitsetContainers;  // NOLINT(readability-identifier-naming)

   uint32_t numberOfValuesStoredInArrayContainers;   // NOLINT(readability-identifier-naming)
   uint32_t numberOfValuesStoredInRunContainers;     // NOLINT(readability-identifier-naming)
   uint32_t numberOfValuesStoredInBitsetContainers;  // NOLINT(readability-identifier-naming)

   uint64_t totalBitmapSizeArrayContainers;   // NOLINT(readability-identifier-naming)
   uint64_t totalBitmapSizeRunContainers;     // NOLINT(readability-identifier-naming)
   uint64_t totalBitmapSizeBitsetContainers;  // NOLINT(readability-identifier-naming)
};

struct BitmapContainerSize {
   uint32_t sectionLength;  // NOLINT(readability-identifier-naming)
   std::map<GENOME_SYMBOL, std::vector<uint32_t>>
      sizePerGenomeSymbolAndSection;  // NOLINT(readability-identifier-naming)

   BitmapContainerSizeStatistic
      bitmapContainerSizeStatistic;  // NOLINT(readability-identifier-naming)

   uint64_t totalBitmapSizeFrozen;    // NOLINT(readability-identifier-naming)
   uint64_t totalBitmapSizeComputed;  // NOLINT(readability-identifier-naming)

   explicit BitmapContainerSize(uint32_t section_length);

   BitmapContainerSize& operator+=(const BitmapContainerSize& other);
};

struct DetailedDatabaseInfo {
   BitmapSizePerSymbol bitmapSizePerSymbol;  // NOLINT(readability-identifier-naming)
   BitmapContainerSize
      bitmapContainerSizePerGenomeSection;  // NOLINT(readability-identifier-naming)
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_API_DATABASE_INFO_H_

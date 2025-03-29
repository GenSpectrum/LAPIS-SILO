#include "silo/api/info_handler.h"

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "silo/api/active_database.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database_info.h"

namespace silo {

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const DatabaseInfo& databaseInfo) {
   json = nlohmann::json{
      {"version", databaseInfo.version},
      {"sequenceCount", databaseInfo.sequence_count},
      {"totalSize", databaseInfo.total_size},
      {"nBitmapsSize", databaseInfo.n_bitmaps_size},
      {"numberOfPartitions", databaseInfo.number_of_partitions}
   };
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const BitmapContainerSizeStatistic& statistics) {
   json = nlohmann::json{
      {"numberOfArrayContainers", statistics.number_of_array_containers},
      {"numberOfRunContainers", statistics.number_of_run_containers},
      {"numberOfBitsetContainers", statistics.number_of_bitset_containers},
      {"numberOfValuesStoredInArrayContainers",
       statistics.number_of_values_stored_in_array_containers},
      {"numberOfValuesStoredInRunContainers", statistics.number_of_values_stored_in_run_containers},
      {"numberOfValuesStoredInBitsetContainers",
       statistics.number_of_values_stored_in_bitset_containers},
      {"totalBitmapSizeArrayContainers", statistics.total_bitmap_size_array_containers},
      {"totalBitmapSizeRunContainers", statistics.total_bitmap_size_run_containers},
      {"totalBitmapSizeBitsetContainers", statistics.total_bitmap_size_bitset_containers}
   };
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const BitmapSizePerSymbol& bitmapSizePerSymbol) {
   std::map<std::string, uint64_t> size_in_bytes_for_nlohmann;
   for (const auto& [symbol, size] : bitmapSizePerSymbol.size_in_bytes) {
      const std::string symbol_string(1, Nucleotide::symbolToChar(symbol));
      size_in_bytes_for_nlohmann[symbol_string] = size;
   }
   json = size_in_bytes_for_nlohmann;
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const BitmapContainerSize& bitmapContainerSize) {
   json = nlohmann::json{
      {"sectionLength", bitmapContainerSize.section_length},
      {"sizePerGenomeSymbolAndSection", bitmapContainerSize.size_per_genome_symbol_and_section},
      {"bitmapContainerSizeStatistic", bitmapContainerSize.bitmap_container_size_statistic},
      {"totalBitmapSizeFrozen", bitmapContainerSize.total_bitmap_size_frozen},
      {"totalBitmapSizeComputed", bitmapContainerSize.total_bitmap_size_computed}
   };
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const DetailedDatabaseInfo& databaseInfo) {
   json = nlohmann::json{
      {"bitmapSizePerSymbol", databaseInfo.sequences.at("main").bitmap_size_per_symbol},
      {"bitmapContainerSizePerGenomeSection",
       databaseInfo.sequences.at("main").bitmap_container_size_per_genome_section}
   };
}

}  // namespace silo

namespace silo::api {

void InfoHandler::get(
   std::shared_ptr<const Database> database,
   crow::request& request,
   crow::response& response
) {
   response.set_header("data-version", database->getDataVersionTimestamp().value);

   const char* details_given = request.url_params.get("details");
   const bool return_detailed_info = details_given && details_given == std::string_view("true");
   const nlohmann::json database_info = return_detailed_info
                                           ? nlohmann::json(database->detailedDatabaseInfo())
                                           : nlohmann::json(database->getDatabaseInfo());
   response.set_header("Content-Type", "application/json");
   response.body = database_info.dump();
   response.end();
}

}  // namespace silo::api

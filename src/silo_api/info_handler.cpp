#include "silo_api/info_handler.h"

#include <map>
#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/database_info.h"
#include "silo_api/database_mutex.h"

namespace silo {

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const DatabaseInfo& databaseInfo) {
   json = nlohmann::json{
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

namespace {
std::map<std::string, std::string> getQueryParameter(const Poco::Net::HTTPServerRequest& request) {
   std::map<std::string, std::string> map;
   const Poco::URI uri(request.getURI());
   const auto query_parameters = uri.getQueryParameters();

   for (const auto& parameter : query_parameters) {
      map.insert(parameter);
   }
   return map;
}
}  // namespace

namespace silo_api {

InfoHandler::InfoHandler(DatabaseMutex& database)
    : database(database) {}

void InfoHandler::get(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto request_parameter = getQueryParameter(request);

   const auto fixed_database = database.getDatabase();

   response.set("data-version", fixed_database.database.getDataVersionTimestamp().value);

   const bool return_detailed_info = request_parameter.find("details") != request_parameter.end() &&
                                     request_parameter.at("details") == "true";
   const nlohmann::json database_info =
      return_detailed_info ? nlohmann::json(database.getDatabase().database.detailedDatabaseInfo())
                           : nlohmann::json(database.getDatabase().database.getDatabaseInfo());
   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << database_info;
}
}  // namespace silo_api

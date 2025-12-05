#pragma once

#include <filesystem>

#include "silo/common/data_version.h"
#include "silo/common/silo_directory.h"
#include "silo/database_info.h"
#include "silo/query_engine/query.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo {

class Database {
  public:
   schema::DatabaseSchema schema;
   std::map<schema::TableName, std::shared_ptr<storage::Table>> tables;

   void updateDataVersion();

  private:
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   Database() = default;

   explicit Database(silo::schema::DatabaseSchema database_schema);

   virtual ~Database() = default;

   void createTable(schema::TableName table_name, silo::schema::TableSchema table_schema);

   void appendData(const schema::TableName& table_name, std::istream& input_stream);

   void query(query_engine::Query query);

   void createNucleotideSequenceTable(
      const std::string& table_name,
      const std::string& primary_key_name,
      const std::string& sequence_name,
      const std::string& reference_sequence,
      const std::vector<std::string>& extra_string_columns = {}
   );

   void createGeneTable(
      const std::string& table_name,
      const std::string& primary_key_name,
      const std::string& sequence_name,
      const std::string& reference_sequence,
      const std::vector<std::string>& extra_string_columns = {}
   );

   void appendDataFromFile(const std::string& table_name, const std::string& file_name);

   void appendDataFromString(const std::string& table_name, std::string json_string);

   void printAllData(const std::string& table_name) const;

   std::string getNucleotideReferenceSequence(
      const std::string& table_name,
      const std::string& sequence_name
   );

   std::string getAminoAcidReferenceSequence(
      const std::string& table_name,
      const std::string& sequence_name
   );

   roaring::Roaring getFilteredBitmap(const std::string& table_name, const std::string& filter);

   template <typename SymbolType>
   [[nodiscard]] std::vector<std::pair<uint64_t, std::string>> getPrevalentMutations(
      const std::string& table_name,
      const std::string& sequence_name,
      double prevalence_threshold,
      const std::string& filter
   ) const;

   [[nodiscard]] std::vector<std::pair<uint64_t, std::string>> getPrevalentNucMutations(
      const std::string& table_name,
      const std::string& sequence_name,
      double prevalence_threshold,
      const std::string& filter
   ) const;

   [[nodiscard]] std::vector<std::pair<uint64_t, std::string>> getPrevalentAminoAcidMutations(
      const std::string& table_name,
      const std::string& sequence_name,
      double prevalence_threshold,
      const std::string& filter
   ) const;

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static std::optional<Database> loadDatabaseStateFromPath(
      const std::filesystem::path& save_directory
   );

   static Database loadDatabaseState(const silo::SiloDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DataVersion::Timestamp getDataVersionTimestamp() const;

   [[nodiscard]] query_engine::QueryPlan createQueryPlan(
      const query_engine::Query& query,
      const config::QueryOptions& query_options,
      std::string_view request_id
   ) const;
};

}  // namespace silo

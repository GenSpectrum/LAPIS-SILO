#include "silo/preprocessing/metadata_info.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace {

std::unordered_map<std::string, std::string> validateFieldsAgainstConfig(
   const std::unordered_map<std::string, std::string>& found_metadata_fields,
   const silo::config::DatabaseConfig& database_config
) {
   std::vector<std::string> config_metadata_fields;
   std::transform(
      database_config.schema.metadata.begin(),
      database_config.schema.metadata.end(),
      std::back_inserter(config_metadata_fields),
      [](auto metadata) { return metadata.name; }
   );

   std::unordered_map<std::string, std::string> validated_metadata_fields;
   for (const auto& [field_name, access_path] : found_metadata_fields) {
      if (std::find(config_metadata_fields.begin(), config_metadata_fields.end(), field_name)
       != config_metadata_fields.end()) {
         validated_metadata_fields.emplace(field_name, access_path);
      }
   }
   for (const std::string& name : config_metadata_fields) {
      if (!validated_metadata_fields.contains(name)) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The metadata field {} which is contained in the database config is "
            "not contained in the input.",
            name
         ));
      }
   }

   std::string metadata_field_string;
   for (const auto& [field_name, select] : validated_metadata_fields) {
      metadata_field_string += "'" + field_name + "' with selection '" + select + "',";
   }
   SPDLOG_TRACE("Found metadata fields: " + metadata_field_string);
   return validated_metadata_fields;
}

void detectInsertionLists(
   const std::filesystem::path& ndjson_file,
   std::unordered_map<std::string, std::string>& metadata_fields_to_validate
) {
   duckdb::DuckDB db(nullptr);
   duckdb::Connection connection(db);
   auto top_level_entries =
      connection.Query(fmt::format("SELECT * FROM '{}' LIMIT 0", ndjson_file.string()));
   for (size_t idx = 0; idx < top_level_entries->ColumnCount(); idx++) {
      const std::string& top_level_entry = top_level_entries->ColumnName(idx);
      if (top_level_entry == "nucleotideInsertions" || top_level_entry == "aminoAcidInsertions") {
         auto contained_insertions = connection.Query(
            fmt::format("SELECT {}.* FROM '{}' LIMIT 0", top_level_entry, ndjson_file.string())
         );
         if (contained_insertions->ColumnCount() == 0) {
            metadata_fields_to_validate[top_level_entry] = "''";
         }
         if (contained_insertions->ColumnCount() == 1) {
            metadata_fields_to_validate[top_level_entry] = fmt::format(
               "list_string_agg({}.{})", top_level_entry, contained_insertions->ColumnName(0)
            );
         }

         std::vector<std::string> list_transforms;
         for (size_t idx2 = 0; idx2 < contained_insertions->ColumnCount(); idx2++) {
            const std::string& sequence_name = contained_insertions->ColumnName(idx2);
            list_transforms.push_back(fmt::format(
               "list_transform({0}.{1}, x ->'{1}:' || x)", top_level_entry, sequence_name
            ));
         }
         metadata_fields_to_validate[top_level_entry] =
            "list_string_agg(flatten([" + boost::join(list_transforms, ",") + "]))";
      }
   }
}

}  // namespace

namespace silo::preprocessing {

MetadataInfo MetadataInfo::validateFromMetadataFile(
   const std::filesystem::path& metadata_file,
   const silo::config::DatabaseConfig& database_config
) {
   if (database_config.schema.metadata.empty()) {
      throw PreprocessingException("Database config without fields not possible");
   }

   duckdb::DuckDB db(nullptr);
   duckdb::Connection connection(db);
   // Get the column names (headers) of the table
   auto result =
      connection.Query(fmt::format("SELECT * FROM '{}' LIMIT 0", metadata_file.string()));

   std::unordered_map<std::string, std::string> file_metadata_fields;
   for (size_t idx = 0; idx < result->ColumnCount(); idx++) {
      file_metadata_fields[result->ColumnName(idx)] = result->ColumnName(idx);
   }
   const std::unordered_map<std::string, std::string> validated_metadata_fields =
      validateFieldsAgainstConfig(file_metadata_fields, database_config);

   MetadataInfo metadata_info;
   metadata_info.database_config = database_config;
   metadata_info.metadata_selects = validated_metadata_fields;
   return metadata_info;
}

MetadataInfo MetadataInfo::validateFromNdjsonFile(
   const std::filesystem::path& ndjson_file,
   const silo::config::DatabaseConfig& database_config
) {
   if (database_config.schema.metadata.empty()) {
      throw PreprocessingException("Database config without fields not possible");
   }

   duckdb::DuckDB db(nullptr);
   duckdb::Connection connection(db);

   auto result = connection.Query(fmt::format(
      "SELECT json_keys(metadata)"
      "FROM "
      "'{}' LIMIT 1; ",
      ndjson_file.string()
   ));
   if (result->HasError()) {
      throw silo::preprocessing::PreprocessingException(
         "Preprocessing exception when retrieving the field 'metadata', "
         "duckdb threw with error: " +
         result->GetError()
      );
   }
   if (result->RowCount() == 0) {
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "File {} is empty, which must not be empty at this point", ndjson_file.string()
      ));
   }
   if (result->RowCount() > 1) {
      throw silo::preprocessing::PreprocessingException(
         "Internal exception, expected Row Count=1, actual " + std::to_string(result->RowCount())
      );
   }

   std::unordered_map<std::string, std::string> metadata_fields_to_validate;
   for (const std::string& metadata_field : preprocessing::extractStringListValue(*result, 0, 0)) {
      metadata_fields_to_validate[metadata_field] = "metadata." + metadata_field;
   }
   detectInsertionLists(ndjson_file, metadata_fields_to_validate);

   const std::unordered_map<std::string, std::string> validated_metadata_fields =
      validateFieldsAgainstConfig(metadata_fields_to_validate, database_config);

   MetadataInfo metadata_info;
   metadata_info.database_config = database_config;
   metadata_info.metadata_selects = validated_metadata_fields;
   return metadata_info;
}

std::vector<std::string> MetadataInfo::getMetadataFields() const {
   std::vector<std::string> ret;
   for (const auto& [field, _] : metadata_selects) {
      ret.push_back(field);
   }
   return ret;
}

std::vector<std::string> MetadataInfo::getMetadataSelects() const {
   std::vector<std::string> ret;
   for (const auto& [field, select] : metadata_selects) {
      ret.push_back(fmt::format("{} as {}", select, field));
   }
   return ret;
}

}  // namespace silo::preprocessing

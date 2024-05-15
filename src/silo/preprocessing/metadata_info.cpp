#include "silo/preprocessing/metadata_info.h"

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
      if (std::find(config_metadata_fields.begin(), config_metadata_fields.end(), field_name) !=
          config_metadata_fields.end()) {
         validated_metadata_fields.emplace(field_name, access_path);
      } else {
         SPDLOG_WARN(
            "Metadata field {} ({}), which is contained in the file is not contained in the "
            "config.",
            field_name,
            access_path
         );
      }
   }
   for (const std::string& name : config_metadata_fields) {
      if (!validated_metadata_fields.contains(name)) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The metadata field '{}' which is contained in the database config is "
            "not contained in the input.",
            name
         ));
      }
   }

   std::string metadata_field_string;
   for (const auto& [field_name, select] : validated_metadata_fields) {
      metadata_field_string += "'";
      metadata_field_string += field_name;
      metadata_field_string += "' with selection '";
      metadata_field_string += select;
      metadata_field_string += "',";
   }
   SPDLOG_TRACE("Found metadata fields: " + metadata_field_string);
   return validated_metadata_fields;
}

}  // namespace

namespace silo::preprocessing {

MetadataInfo::MetadataInfo(std::unordered_map<std::string, std::string> metadata_selects)
    : metadata_selects(std::move(metadata_selects)) {}

MetadataInfo MetadataInfo::validateFromMetadataFile(
   const std::filesystem::path& metadata_file,
   const silo::config::DatabaseConfig& database_config
) {
   duckdb::DuckDB duck_db(nullptr);
   duckdb::Connection connection(duck_db);
   // Get the column names (headers) of the table
   auto result =
      connection.Query(fmt::format("SELECT * FROM '{}' LIMIT 0", metadata_file.string()));

   std::unordered_map<std::string, std::string> file_metadata_fields;
   for (size_t idx = 0; idx < result->ColumnCount(); idx++) {
      file_metadata_fields[result->ColumnName(idx)] = "\"" + result->ColumnName(idx) + "\"";
   }
   const std::unordered_map<std::string, std::string> validated_metadata_fields =
      validateFieldsAgainstConfig(file_metadata_fields, database_config);

   return {validated_metadata_fields};
}

MetadataInfo MetadataInfo::validateFromNdjsonFile(
   const std::filesystem::path& ndjson_file,
   const silo::config::DatabaseConfig& database_config
) {
   duckdb::DuckDB duck_db(nullptr);
   duckdb::Connection connection(duck_db);

   auto result = connection.Query(fmt::format(
      "SELECT json_keys(metadata) "
      "FROM read_json_auto(\"{}\") LIMIT 1; ",
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
      metadata_fields_to_validate[metadata_field] = "metadata.\"" + metadata_field + "\"";
   }

   const std::unordered_map<std::string, std::string> validated_metadata_fields =
      validateFieldsAgainstConfig(metadata_fields_to_validate, database_config);

   return {validated_metadata_fields};
}

std::vector<std::string> MetadataInfo::getMetadataFields() const {
   std::vector<std::string> ret;
   ret.reserve(metadata_selects.size());
   for (const auto& [field, _] : metadata_selects) {
      ret.push_back("\"" + field + "\"");
   }
   return ret;
}

std::vector<std::string> MetadataInfo::getMetadataSelects() const {
   std::vector<std::string> ret;
   ret.reserve(metadata_selects.size());
   for (const auto& [field, select] : metadata_selects) {
      ret.push_back(fmt::format(R"({} AS "{}")", select, field));
   }
   return ret;
}

}  // namespace silo::preprocessing

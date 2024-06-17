#include "silo/preprocessing/metadata_info.h"

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace {
using silo::config::ValueType;

std::string toSQLType(ValueType value_type) {
   switch (value_type) {
      case ValueType::INT:
         return "INT4";
      case ValueType::STRING:
      case ValueType::PANGOLINEAGE:
         return "VARCHAR";
      case ValueType::FLOAT:
         return "FLOAT4";
      case ValueType::BOOL:
         return "BOOL";
      case ValueType::DATE:
         return "DATE";
   }
   abort();
}

}  // namespace

namespace silo::preprocessing {

void MetadataInfo::validateMetadataFile(
   const std::filesystem::path& metadata_file,
   const silo::config::DatabaseConfig& database_config
) {
   duckdb::DuckDB duck_db(nullptr);
   duckdb::Connection connection(duck_db);
   // Get the column names (headers) of the table
   auto result = connection.Query(fmt::format(
      "SELECT * FROM read_csv_auto('{}', delim = '\t', header = true) LIMIT 0",
      metadata_file.string()
   ));

   if (result->HasError()) {
      const std::string error_message = fmt::format(
         "Preprocessing exception when retrieving the fields of the "
         "metadata file '{}', "
         "duckdb threw with error: {}",
         metadata_file.string(),
         result->GetError()
      );
      SPDLOG_ERROR(error_message);
      throw silo::preprocessing::PreprocessingException(error_message);
   }

   std::set<std::string> actual_fields;
   for (size_t idx = 0; idx < result->ColumnCount(); idx++) {
      actual_fields.emplace(result->ColumnName(idx));
      if (std::find_if(database_config.schema.metadata.begin(), database_config.schema.metadata.end(), [&](const auto& metadata) {
             return metadata.name == result->ColumnName(idx);
          }) == database_config.schema.metadata.end()) {
         SPDLOG_WARN(
            "The field '{}' which is contained in the metadata file '{}' is not contained in the "
            "database config.",
            result->ColumnName(idx),
            metadata_file.string()
         );
      }
   }

   for (const auto& field : database_config.schema.metadata) {
      if (!actual_fields.contains(field.name)) {
         const std::string error_message = fmt::format(
            "The field '{}' which is contained in the database config is not contained in the "
            "input field '{}'.",
            field.name,
            metadata_file.string()
         );
         SPDLOG_ERROR(error_message);
         throw silo::preprocessing::PreprocessingException(error_message);
      }
   }
}

bool MetadataInfo::isNdjsonFileEmpty(const std::filesystem::path& ndjson_file) {
   duckdb::DuckDB duck_db(nullptr);
   duckdb::Connection connection(duck_db);

   auto result = connection.Query(fmt::format(
      "SELECT COUNT(*) "
      "FROM (SELECT * FROM read_json_auto(\"{}\") LIMIT 1);",
      ndjson_file.string()
   ));

   auto row_count_value = result->GetValue<int64_t>(0, 0);
   const int64_t row_count = duckdb::BigIntValue::Get(row_count_value);
   return row_count == 0;
}

void MetadataInfo::validateNdjsonFile(
   const std::filesystem::path& ndjson_file,
   const silo::config::DatabaseConfig& database_config
) {
   duckdb::DuckDB duck_db(nullptr);
   duckdb::Connection connection(duck_db);

   auto result = connection.Query(fmt::format(
      "SELECT metadata.* "
      "FROM read_json_auto(\"{}\") LIMIT 0; ",
      ndjson_file.string()
   ));

   if (result->HasError()) {
      const std::string error_message = fmt::format(
         "Preprocessing exception when retrieving the fields of the struct 'metadata' from the "
         "metadata ndjson file '{}', "
         "duckdb threw with error: {}",
         ndjson_file.string(),
         result->GetError()
      );
      SPDLOG_ERROR(error_message);
      throw PreprocessingException(error_message);
   }

   std::set<std::string> actual_fields;
   for (size_t idx = 0; idx < result->ColumnCount(); idx++) {
      actual_fields.emplace(result->ColumnName(idx));
      if (std::find_if(database_config.schema.metadata.begin(), database_config.schema.metadata.end(), [&](const auto& metadata) {
             return metadata.name == result->ColumnName(idx);
          }) == database_config.schema.metadata.end()) {
         SPDLOG_WARN(
            "The field '{}' which is contained in the metadata file '{}' is not contained in the "
            "database config.",
            result->ColumnName(idx),
            ndjson_file.string()
         );
      }
   }

   for (const auto& field : database_config.schema.metadata) {
      if (!actual_fields.contains(field.name)) {
         const std::string error_message = fmt::format(
            "The field '{}' which is contained in the database config is not contained in the "
            "input field '{}'.",
            field.name,
            ndjson_file.string()
         );
         SPDLOG_ERROR(error_message);
         throw silo::preprocessing::PreprocessingException(error_message);
      }
   }
}

std::vector<std::string> MetadataInfo::getMetadataFields(
   const silo::config::DatabaseConfig& database_config
) {
   std::vector<std::string> ret;
   ret.reserve(database_config.schema.metadata.size());
   for (const auto& field : database_config.schema.metadata) {
      ret.push_back("\"" + field.name + "\"");
   }
   return ret;
}

std::vector<std::string> MetadataInfo::getMetadataSQLTypes(
   const silo::config::DatabaseConfig& database_config
) {
   std::vector<std::string> ret;
   ret.reserve(database_config.schema.metadata.size());
   for (const auto& field : database_config.schema.metadata) {
      ret.push_back(fmt::format("\"{}\" {}", field.name, toSQLType(field.type)));
   }
   return ret;
}

std::vector<std::string> MetadataInfo::getMetadataSelects(
   const silo::config::DatabaseConfig& database_config
) {
   std::vector<std::string> ret;
   ret.reserve(database_config.schema.metadata.size());
   for (const auto& field : database_config.schema.metadata) {
      ret.push_back(fmt::format(R"( "metadata"."{0}" AS "{0}")", field.name));
   }
   return ret;
}

}  // namespace silo::preprocessing

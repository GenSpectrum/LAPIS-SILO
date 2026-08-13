#include "silo/preprocessing/preprocessing.h"

#include "silo/append/append_exception.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/initialize/initialize_exception.h"
#include "silo/initialize/initializer.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/schema/duplicate_primary_key_exception.h"

namespace rhydb::preprocessing {

Database preprocessing(const config::PreprocessingConfig& preprocessing_config) {
   try {
      SPDLOG_INFO("preprocessing - initializing Database");
      Database database;
      initialize::Initializer::createTableInDatabase(
         schema::TableName::getDefault(), preprocessing_config.initialization_files, database
      );

      SPDLOG_INFO("preprocessing - successfully initialized Database, now opening input");
      auto input = InputStreamWrapper::openFileOrStdIn(preprocessing_config.getInputFilePath());

      SPDLOG_INFO("preprocessing - appending data to Database");
      database.appendData(schema::TableName::getDefault(), input.getInputStream());

      SPDLOG_INFO("preprocessing - validating primary key uniqueness");
      database.tables.at(schema::TableName::getDefault())->validatePrimaryKeyUnique();

      SPDLOG_INFO("preprocessing - finished preprocessing");
      return database;
   } catch (const initialize::InitializeException& exception) {
      throw PreprocessingException(
         "preprocessing - exception when initializing database: {}", exception.what()
      );
   } catch (const append::AppendException& exception) {
      throw PreprocessingException(
         "preprocessing - exception when appending data: {}", exception.what()
      );
   } catch (const schema::DuplicatePrimaryKeyException& exception) {
      throw PreprocessingException(
         "preprocessing - primary key uniqueness validation failed: {}", exception.what()
      );
   }
}

}  // namespace rhydb::preprocessing

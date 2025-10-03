#include "silo/preprocessing/preprocessing.h"

#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/initialize/initialize_exception.h"
#include "silo/initialize/initializer.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

Database preprocessing(const config::PreprocessingConfig& preprocessing_config) {
   try {
      SPDLOG_INFO("preprocessing - initializing Database");
      auto database =
         initialize::Initializer::initializeDatabase(preprocessing_config.initialization_files);

      SPDLOG_INFO("preprocessing - successfully initialized Database, now opening input");
      auto input = InputStreamWrapper::openFileOrStdIn(preprocessing_config.getInputFilePath());

      SPDLOG_INFO("preprocessing - appending data to Database");
      auto input_data = append::NdjsonLineReader{input.getInputStream()};
      append::appendDataToDatabase(database, input_data);

      SPDLOG_INFO("preprocessing - finished preprocessing");
      return database;
   } catch (const initialize::InitializeException& exception) {
      throw preprocessing::PreprocessingException(
         "preprocessing - exception when initializing database: {}", exception.what()
      );
   } catch (const append::AppendException& exception) {
      throw preprocessing::PreprocessingException(
         "preprocessing - exception when appending data: {}", exception.what()
      );
   }
}

}  // namespace silo::preprocessing

#include "silo/preprocessing/preprocessing.h"

#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/initialize/initialize_exception.h"
#include "silo/initialize/initializer.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

Database preprocessing(const config::PreprocessingConfig& preprocessing_config) {
   SPDLOG_INFO("preprocessing - initializing Database");
   auto database =
      silo::initialize::Initializer::initializeDatabase(preprocessing_config.initialization_files);

   SPDLOG_INFO("preprocessing - successfully initialized Database, now opening input");
   auto input = silo::InputStreamWrapper::openFileOrStdIn(preprocessing_config.getInputFilePath());

   try {
      SPDLOG_INFO("preprocessing - appending data to Database");
      auto input_data = silo::append::NdjsonLineReader{input.getInputStream()};
      silo::append::appendDataToDatabase(database, input_data);
   } catch (const silo::append::AppendException& exception) {
      throw silo::preprocessing::PreprocessingException(
         "preprocessing - exception when appending data: {}", exception.what()
      );
   }

   SPDLOG_INFO("preprocessing - finished preprocessing");
   return database;
}

}  // namespace silo::preprocessing

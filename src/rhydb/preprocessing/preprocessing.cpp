#include "rhydb/preprocessing/preprocessing.h"

#include "rhydb/append/append_exception.h"
#include "rhydb/append/bam/bam_exception.h"
#include "rhydb/append/fasta/fasta_exception.h"
#include "rhydb/append/ndjson_line_reader.h"
#include "rhydb/common/input_stream_wrapper.h"
#include "rhydb/initialize/initialize_exception.h"
#include "rhydb/initialize/initializer.h"
#include "rhydb/preprocessing/preprocessing_exception.h"
#include "rhydb/schema/duplicate_primary_key_exception.h"

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

Database preprocessingBam(
   const config::PreprocessingConfig& preprocessing_config,
   append::bam::BamIngestOptions options
) {
   try {
      SPDLOG_INFO("preprocessing (BAM) - initializing Database");
      Database database;
      initialize::Initializer::createTableInDatabase(
         schema::TableName::getDefault(), preprocessing_config.initialization_files, database
      );

      SPDLOG_INFO("preprocessing (BAM) - successfully initialized Database, now opening input");
      auto input = InputStreamWrapper::openFileOrStdIn(preprocessing_config.getInputFilePath());

      SPDLOG_INFO("preprocessing (BAM) - appending BAM reads to Database");
      database.appendBamData(schema::TableName::getDefault(), input.getInputStream(), options);

      SPDLOG_INFO("preprocessing (BAM) - validating primary key uniqueness");
      database.tables.at(schema::TableName::getDefault())->validatePrimaryKeyUnique();

      SPDLOG_INFO("preprocessing (BAM) - finished preprocessing");
      return database;
   } catch (const initialize::InitializeException& exception) {
      throw PreprocessingException(
         "preprocessing (BAM) - exception when initializing database: {}", exception.what()
      );
   } catch (const append::bam::BamException& exception) {
      throw PreprocessingException(
         "preprocessing (BAM) - exception when reading the BAM input: {}", exception.what()
      );
   } catch (const append::AppendException& exception) {
      throw PreprocessingException(
         "preprocessing (BAM) - exception when appending data: {}", exception.what()
      );
   } catch (const schema::DuplicatePrimaryKeyException& exception) {
      throw PreprocessingException(
         "preprocessing (BAM) - primary key uniqueness validation failed: {}", exception.what()
      );
   }
}

Database preprocessingFasta(const config::PreprocessingConfig& preprocessing_config) {
   try {
      SPDLOG_INFO("preprocessing (FASTA) - initializing Database");
      Database database;
      initialize::Initializer::createTableInDatabase(
         schema::TableName::getDefault(), preprocessing_config.initialization_files, database
      );

      SPDLOG_INFO("preprocessing (FASTA) - successfully initialized Database, now opening input");
      auto input = InputStreamWrapper::openFileOrStdIn(preprocessing_config.getInputFilePath());

      SPDLOG_INFO("preprocessing (FASTA) - appending FASTA records to Database");
      database.appendFastaData(schema::TableName::getDefault(), input.getInputStream());

      SPDLOG_INFO("preprocessing (FASTA) - validating primary key uniqueness");
      database.tables.at(schema::TableName::getDefault())->validatePrimaryKeyUnique();

      SPDLOG_INFO("preprocessing (FASTA) - finished preprocessing");
      return database;
   } catch (const initialize::InitializeException& exception) {
      throw PreprocessingException(
         "preprocessing (FASTA) - exception when initializing database: {}", exception.what()
      );
   } catch (const append::fasta::FastaException& exception) {
      throw PreprocessingException(
         "preprocessing (FASTA) - exception when reading the FASTA input: {}", exception.what()
      );
   } catch (const append::AppendException& exception) {
      throw PreprocessingException(
         "preprocessing (FASTA) - exception when appending data: {}", exception.what()
      );
   } catch (const schema::DuplicatePrimaryKeyException& exception) {
      throw PreprocessingException(
         "preprocessing (FASTA) - primary key uniqueness validation failed: {}", exception.what()
      );
   }
}

}  // namespace rhydb::preprocessing

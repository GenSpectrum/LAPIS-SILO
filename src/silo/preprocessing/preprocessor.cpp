#include "silo/preprocessing/preprocessor.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <silo/zstdfasta/zstdfasta_table_reader.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/block_timer.h"
#include "silo/common/fasta_reader.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/metadata_info.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sequence_info.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/unaligned_sequence_store.h"
#include "silo/zstdfasta/zstd_decompressor.h"
#include "silo/zstdfasta/zstdfasta_table.h"

namespace silo::preprocessing {

constexpr std::string_view FASTA_EXTENSION = ".fasta";

Preprocessor::Preprocessor(
   config::PreprocessingConfig preprocessing_config_,
   config::DatabaseConfig database_config_,
   const ReferenceGenomes& reference_genomes,
   PangoLineageAliasLookup alias_lookup
)
    : preprocessing_config(std::move(preprocessing_config_)),
      database_config(std::move(database_config_)),
      preprocessing_db(preprocessing_config.getPreprocessingDatabaseLocation(), reference_genomes),
      reference_genomes_(reference_genomes),
      alias_lookup_(std::move(alias_lookup)) {}

Database Preprocessor::preprocess() {
   SPDLOG_INFO(
      "preprocessing - creating intermediate results directory '{}'",
      preprocessing_config.getIntermediateResultsDirectory().string()
   );
   std::filesystem::create_directory(preprocessing_config.getIntermediateResultsDirectory());
   if (!std::filesystem::is_directory(preprocessing_config.getIntermediateResultsDirectory())) {
      auto error = fmt::format(
         "Directory for intermediate results could not be created.",
         preprocessing_config.getIntermediateResultsDirectory().string()
      );
      SPDLOG_ERROR(error);
      throw silo::preprocessing::PreprocessingException(error);
   }

   const auto& ndjson_input_filename = preprocessing_config.getNdjsonInputFilename();
   if (ndjson_input_filename.has_value()) {
      SPDLOG_INFO("preprocessing - ndjson pipeline chosen");
      SPDLOG_DEBUG(
         "preprocessing - building preprocessing tables from ndjson input '{}'",
         ndjson_input_filename.value().string()
      );
      buildTablesFromNdjsonInput(ndjson_input_filename.value());
      SPDLOG_DEBUG("preprocessing - building partitioning tables");
      buildPartitioningTable();
      SPDLOG_DEBUG("preprocessing - creating compressed sequence views for building SILO");
      createPartitionedSequenceTablesFromNdjson(ndjson_input_filename.value());
   } else {
      SPDLOG_INFO("preprocessing - classic metadata file pipeline chosen");
      SPDLOG_DEBUG(
         "preprocessing - building metadata tables from metadata input '{}'",
         preprocessing_config.getMetadataInputFilename()->string()
      );
      buildMetadataTableFromFile(*preprocessing_config.getMetadataInputFilename());
      SPDLOG_DEBUG("preprocessing - building partitioning tables");
      buildPartitioningTable();
      SPDLOG_DEBUG("preprocessing - creating partitioned sequence tables for building SILO");
      createPartitionedSequenceTablesFromSequenceFiles();
   }
   SPDLOG_INFO("preprocessing - finished initial loading of data");

   const auto partition_descriptor = preprocessing_db.getPartitionDescriptor();

   std::string order_by_clause = database_config.schema.getStrictOrderByClause();
   SPDLOG_INFO("preprocessing - order by clause is {}", order_by_clause);

   SPDLOG_INFO("preprocessing - building database");
   preprocessing_db.refreshConnection();
   return buildDatabase(
      partition_descriptor, order_by_clause, preprocessing_config.getIntermediateResultsDirectory()
   );
}

void Preprocessor::buildTablesFromNdjsonInput(const std::filesystem::path& file_name) {
   if (!std::filesystem::exists(file_name)) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The specified input file {} does not exist.", file_name.string())
      );
   }
   if (std::filesystem::is_empty(file_name)) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The specified input file {} is empty.", file_name.string())
      );
   }

   SPDLOG_DEBUG("build - validating metadata file '{}' with config", file_name.string());
   const auto metadata_info = MetadataInfo::validateFromNdjsonFile(file_name, database_config);

   (void)preprocessing_db.query(fmt::format(
      R"-(
         CREATE OR REPLACE TABLE metadata_table AS
         SELECT {}
         FROM '{}';
      )-",
      boost::join(metadata_info.getMetadataSelects(), ","),
      file_name.string()
   ));

   auto null_primary_key_result = preprocessing_db.query(fmt::format(
      R"-(
         SELECT {0} FROM metadata_table
         WHERE {0} IS NULL;
      )-",
      database_config.schema.primary_key
   ));
   if (null_primary_key_result->RowCount() > 0) {
      const std::string error_message = fmt::format(
         "Error, there are {} primary keys that are NULL",
         null_primary_key_result->RowCount(),
         file_name.string()
      );
      SPDLOG_ERROR(error_message);
      if (null_primary_key_result->RowCount() <= 10) {
         SPDLOG_ERROR(null_primary_key_result->ToString());
      }
      throw silo::preprocessing::PreprocessingException(error_message);
   }
}

void Preprocessor::buildMetadataTableFromFile(const std::filesystem::path& metadata_filename) {
   const MetadataInfo metadata_info =
      MetadataInfo::validateFromMetadataFile(metadata_filename, database_config);

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE metadata_table AS\n"
      "SELECT {}\n"
      "FROM '{}';",
      boost::join(metadata_info.getMetadataSelects(), ","),
      metadata_filename.string()
   ));
}

void Preprocessor::buildPartitioningTable() {
   if (database_config.schema.partition_by.has_value()) {
      SPDLOG_DEBUG(
         "preprocessing - partitioning input by metadata key '{}'",
         database_config.schema.partition_by.value()
      );
      buildPartitioningTableByColumn(database_config.schema.partition_by.value());
   } else {
      SPDLOG_DEBUG("preprocessing - no metadata key for partitioning provided");
      buildEmptyPartitioning();
   }
}

void Preprocessor::buildPartitioningTableByColumn(const std::string& partition_by_field) {
   SPDLOG_DEBUG("preprocessing - calculating partitions");

   (void)preprocessing_db.query(fmt::format(
      R"-(
CREATE OR REPLACE TABLE partition_keys AS
SELECT row_number() OVER () - 1 AS id, partition_key, count
FROM (SELECT {} AS partition_key, COUNT(*) AS count
      FROM metadata_table
      GROUP BY partition_key
      ORDER BY partition_key);
)-",
      partition_by_field
   ));

   // create Recursive Hierarchical Partitioning By Partition Field
   (void)preprocessing_db.query(
      R"-(
CREATE OR REPLACE TABLE partitioning AS
WITH RECURSIVE
          allowed_count(allowed_count) AS (SELECT sum(count) / 32 FROM partition_keys),
          grouped_partition_keys(from_id, to_id, count) AS
              (SELECT id, id, count
               FROM partition_keys
               WHERE id = 0
               UNION ALL
               SELECT CASE WHEN l1.count <= allowed_count THEN l1.from_id ELSE l2.id END,
                      l2.id,
                      CASE WHEN l1.count <= allowed_count
                           THEN l1.count + l2.count
                           ELSE l2.count END
               FROM grouped_partition_keys l1,
                    partition_keys l2,
                    allowed_count
WHERE l1.to_id + 1 = l2.id)
SELECT row_number() OVER () - 1 AS partition_id, from_id, to_id, count
FROM (SELECT from_id, MAX(to_id) AS to_id, MAX(count) AS count
      FROM grouped_partition_keys
      GROUP BY from_id)
)-"
   );

   (void)preprocessing_db.query(
      R"-(
CREATE OR REPLACE TABLE partition_key_to_partition AS
SELECT partition_keys.partition_key AS partition_key,
  partitioning.partition_id AS partition_id
FROM partition_keys,
     partitioning
WHERE partition_keys.id >= partitioning.from_id
  AND partition_keys.id <= partitioning.to_id;
)-"
   );

   (void)preprocessing_db.query(fmt::format(
      R"-(
CREATE OR REPLACE VIEW partitioned_metadata AS
SELECT partitioning.partition_id AS partition_id, metadata_table.*
FROM partition_keys,
     partitioning,
     metadata_table
WHERE (metadata_table.{0} = partition_keys.partition_key OR (metadata_table.{0} IS NULL
AND partition_keys.partition_key IS NULL))
  AND partition_keys.id >= partitioning.from_id
  AND partition_keys.id <= partitioning.to_id;
)-",
      partition_by_field
   ));
}

void Preprocessor::buildEmptyPartitioning() {
   SPDLOG_INFO(
      "preprocessing - skip partition merging because no partition_by key was provided, instead "
      "putting all sequences into the same partition"
   );

   (void)preprocessing_db.query(
      R"-(
CREATE OR REPLACE TABLE partitioning AS
SELECT 0::bigint AS partition_id, 0::bigint AS from_id, 0::bigint AS to_id, count(*) AS count
FROM metadata_table;
)-"
   );

   (void)preprocessing_db.query(
      "CREATE OR REPLACE TABLE partition_key_to_partition AS\n"
      "SELECT 0::bigint AS partition_key, 0::bigint AS partition_id;"
   );

   (void)preprocessing_db.query(
      "CREATE OR REPLACE VIEW partitioned_metadata AS\n"
      "SELECT 0::bigint AS partition_id, metadata_table.*\n"
      "FROM metadata_table;"
   );
}

void Preprocessor::createPartitionedSequenceTablesFromNdjson(const std::filesystem::path& file_name
) {
   const SequenceInfo sequence_info(reference_genomes_);
   sequence_info.validate(preprocessing_db.getConnection(), file_name);

   std::string partition_by_select;
   std::string partition_by_where;
   if (database_config.schema.partition_by.has_value()) {
      partition_by_select = "partition_key_to_partition.partition_id AS partition_id";
      partition_by_where = fmt::format(
         "WHERE (metadata.\"{0}\" = partition_key_to_partition.partition_key) OR "
         "(metadata.\"{0}\" IS NULL AND "
         "partition_key_to_partition.partition_key IS NULL)",
         database_config.schema.partition_by.value()
      );
   } else {
      partition_by_select = "0 AS partition_id";
      partition_by_where = "";
   }

   createUnalignedPartitionedSequenceFiles(file_name, partition_by_select, partition_by_where);

   createAlignedPartitionedSequenceViews(
      file_name, sequence_info, partition_by_select, partition_by_where
   );
}

void Preprocessor::createAlignedPartitionedSequenceViews(
   const std::filesystem::path& file_name,
   const SequenceInfo& sequence_info,
   const std::string& partition_by_select,
   const std::string& partition_by_where
) {
   std::string order_by_select = ", metadata.\"" + database_config.schema.primary_key + "\" AS \"" +
                                 database_config.schema.primary_key + "\"";
   std::string order_by_fields = ", \"" + database_config.schema.primary_key + "\"";
   if (database_config.schema.date_to_sort_by.has_value()) {
      order_by_select += ", metadata.\"" + database_config.schema.date_to_sort_by.value() +
                         "\" AS \"" + database_config.schema.date_to_sort_by.value() + "\"";
      order_by_fields += ", \"" + database_config.schema.date_to_sort_by.value() + "\"";
   }

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE sequence_table AS\n"
      "SELECT metadata.\"{}\" AS key, {},"
      "{}"
      "{} \n"
      "FROM '{}', partition_key_to_partition "
      "{};",
      database_config.schema.primary_key,
      boost::join(sequence_info.getAlignedSequenceSelects(preprocessing_db), ","),
      partition_by_select,
      order_by_select,
      file_name.string(),
      partition_by_where
   ));

   for (const auto& [seq_name, _] : reference_genomes_.raw_nucleotide_sequences) {
      (void)preprocessing_db.query(fmt::format(
         "CREATE OR REPLACE VIEW nuc_{0} AS\n"
         "SELECT key, nuc_{0} AS sequence, partition_id"
         "{1}"
         "FROM sequence_table;",
         seq_name,
         order_by_fields
      ));
   }

   for (const auto& [seq_name, _] : reference_genomes_.raw_aa_sequences) {
      (void)preprocessing_db.query(fmt::format(
         "CREATE OR REPLACE VIEW gene_{0} AS\n"
         "SELECT key, gene_{0} AS sequence, partition_id"
         "{1}"
         "FROM sequence_table;",
         seq_name,
         order_by_fields
      ));
   }
}

void Preprocessor::createUnalignedPartitionedSequenceFiles(
   const std::filesystem::path& file_name,
   const std::string& partition_by_select,
   const std::string& partition_by_where
) {
   for (const auto& [seq_name, _] : reference_genomes_.raw_nucleotide_sequences) {
      const std::string table_sql = fmt::format(
         "SELECT metadata.\"{}\" AS key, {},"
         "{} \n"
         "FROM '{}', partition_key_to_partition "
         "{}",
         database_config.schema.primary_key,
         SequenceInfo::getUnalignedSequenceSelect(seq_name, preprocessing_db),
         partition_by_select,
         file_name.string(),
         partition_by_where
      );
      createUnalignedPartitionedSequenceFile(seq_name, table_sql);
   }
}

void Preprocessor::createUnalignedPartitionedSequenceFile(
   const std::string& seq_name,
   const std::string& table_sql
) {
   const std::filesystem::path save_location =
      preprocessing_config.getIntermediateResultsDirectory() / ("unaligned_nuc_" + seq_name);
   preprocessing_db.query(fmt::format(
      "COPY ({}) TO '{}' (FORMAT PARQUET, PARTITION_BY ({}), OVERWRITE_OR_IGNORE);",
      table_sql,
      save_location.string(),
      "partition_id"
   ));
   preprocessing_db.query("VACUUM;");
}

void Preprocessor::createPartitionedSequenceTablesFromSequenceFiles() {
   for (const auto& [sequence_name, reference_sequence] :
        reference_genomes_.raw_nucleotide_sequences) {
      createPartitionedTableForSequence(
         sequence_name,
         reference_sequence,
         preprocessing_config.getNucFilenameNoExtension(sequence_name)
            .replace_extension(FASTA_EXTENSION),
         "nuc_"
      );

      preprocessing_db.generateSequenceTableFromFasta(
         "unaligned_tmp",
         reference_sequence,
         preprocessing_config.getUnalignedNucFilenameNoExtension(sequence_name)
            .replace_extension(FASTA_EXTENSION)
      );
      createUnalignedPartitionedSequenceFile(
         sequence_name,
         fmt::format(
            "SELECT unaligned_tmp.key AS key, unaligned_tmp.sequence AS unaligned_nuc_{}, "
            "partitioned_metadata.partition_id AS partition_id "
            "FROM unaligned_tmp RIGHT JOIN partitioned_metadata "
            "ON unaligned_tmp.key = partitioned_metadata.\"{}\" ",
            sequence_name,
            database_config.schema.primary_key
         )
      );
      preprocessing_db.query("DROP TABLE IF EXISTS unaligned_tmp;");
   }

   for (const auto& [sequence_name, reference_sequence] : reference_genomes_.raw_aa_sequences) {
      createPartitionedTableForSequence(
         sequence_name,
         reference_sequence,
         preprocessing_config.getGeneFilenameNoExtension(sequence_name)
            .replace_extension(FASTA_EXTENSION),
         "gene_"
      );
   }
}

void Preprocessor::createPartitionedTableForSequence(
   const std::string& sequence_name,
   const std::string& reference_sequence,
   const std::filesystem::path& filename,
   const std::string& table_prefix
) {
   std::string order_by_select = ", raw.key AS " + database_config.schema.primary_key;
   if (database_config.schema.date_to_sort_by.has_value()) {
      order_by_select += ", partitioned_metadata." +
                         database_config.schema.date_to_sort_by.value() + " AS " +
                         database_config.schema.date_to_sort_by.value();
   }

   const std::string raw_table_name = "raw_" + table_prefix + sequence_name;
   const std::string table_name = table_prefix + sequence_name;

   preprocessing_db.generateSequenceTableFromFasta(raw_table_name, reference_sequence, filename);

   (void)preprocessing_db.query(fmt::format(
      R"-(
         CREATE OR REPLACE VIEW {} AS
         SELECT key, sequence,
         partitioned_metadata.partition_id AS partition_id
         {}
         FROM {} AS raw RIGHT JOIN partitioned_metadata
         ON raw.key = partitioned_metadata."{}";
      )-",
      table_name,
      order_by_select,
      raw_table_name,
      database_config.schema.primary_key
   ));
}

Database Preprocessor::buildDatabase(
   const preprocessing::Partitions& partition_descriptor,
   const std::string& order_by_clause,
   const std::filesystem::path& intermediate_results_directory
) {
   Database database;
   database.database_config = database_config;
   database.alias_key = alias_lookup_;
   database.intermediate_results_directory = intermediate_results_directory;
   const DataVersion& data_version = DataVersion::mineDataVersion();
   SPDLOG_INFO("preprocessing - mining data data_version: {}", data_version.toString());
   database.setDataVersion(data_version);

   int64_t micros = 0;
   {
      const silo::common::BlockTimer timer(micros);
      for (const auto& partition : partition_descriptor.getPartitions()) {
         database.partitions.emplace_back(partition.getPartitionChunks());
      }
      database.initializeColumns();
      database.initializeNucSequences(reference_genomes_.nucleotide_sequences);
      database.initializeAASequences(reference_genomes_.aa_sequences);

      tbb::task_group tasks;

      tasks.run([&]() {
         SPDLOG_INFO("build - building metadata store in parallel");

         buildMetadataStore(database, partition_descriptor, order_by_clause);

         SPDLOG_INFO("build - finished metadata store");
      });

      tasks.run([&]() {
         SPDLOG_INFO("build - building nucleotide sequence stores");
         buildNucleotideSequenceStore(database, partition_descriptor, order_by_clause);
         SPDLOG_INFO("build - finished nucleotide sequence stores");

         SPDLOG_INFO("build - building amino acid sequence stores");
         buildAminoAcidSequenceStore(database, partition_descriptor, order_by_clause);
         SPDLOG_INFO("build - finished amino acid sequence stores");
      });

      tasks.wait();

      SPDLOG_INFO("build - finalizing insertion indexes");
      database.finalizeInsertionIndexes();
   }

   SPDLOG_INFO("Build took {}", silo::common::formatDuration(micros));
   SPDLOG_INFO("database info: {}", database.getDatabaseInfo());

   database.validate();

   return database;
}

void Preprocessor::buildMetadataStore(
   Database& database,
   const preprocessing::Partitions& partition_descriptor,
   const std::string& order_by_clause
) {
   for (size_t partition_id = 0; partition_id < partition_descriptor.getPartitions().size();
        ++partition_id) {
      const auto& part = partition_descriptor.getPartitions()[partition_id];
      for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size(); ++chunk_index) {
         const uint32_t sequences_added =
            database.partitions.at(partition_id)
               .columns.fill(
                  preprocessing_db.getConnection(), partition_id, order_by_clause, database_config
               );
         database.partitions.at(partition_id).sequence_count += sequences_added;
      }
      SPDLOG_INFO("build - finished columns for partition {}", partition_id);
   }
}

void Preprocessor::buildNucleotideSequenceStore(
   Database& database,
   const preprocessing::Partitions& partition_descriptor,
   const std::string& order_by_clause
) {
   for (const auto& pair : reference_genomes_.raw_nucleotide_sequences) {
      const std::string& nuc_name = pair.first;
      const std::string& reference_sequence = pair.second;
      tbb::parallel_for(
         tbb::blocked_range<size_t>(0, partition_descriptor.getPartitions().size()),
         [&](const auto& local) {
            for (auto partition_index = local.begin(); partition_index != local.end();
                 ++partition_index) {
               const auto& part = partition_descriptor.getPartitions()[partition_index];
               for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size();
                    ++chunk_index) {
                  SPDLOG_DEBUG(
                     "build - building aligned sequence store for nucleotide "
                     "sequence {} and partition {}",
                     nuc_name,
                     partition_index
                  );

                  silo::ZstdFastaTableReader sequence_input(
                     preprocessing_db.getConnection(),
                     "nuc_" + nuc_name,
                     reference_sequence,
                     "sequence",
                     fmt::format("partition_id = {}", partition_index),
                     order_by_clause
                  );
                  database.partitions.at(partition_index)
                     .nuc_sequences.at(nuc_name)
                     .fill(sequence_input);
               }
            }
         }
      );
      SPDLOG_INFO("build - finished nucleotide sequence {}", nuc_name);
   }
}

void Preprocessor::buildAminoAcidSequenceStore(
   silo::Database& database,
   const preprocessing::Partitions& partition_descriptor,
   const std::string& order_by_clause
) {
   for (const auto& pair : reference_genomes_.raw_aa_sequences) {
      const std::string& aa_name = pair.first;
      const std::string& reference_sequence = pair.second;
      tbb::parallel_for(
         tbb::blocked_range<size_t>(0, partition_descriptor.getPartitions().size()),
         [&](const auto& local) {
            for (auto partition_index = local.begin(); partition_index != local.end();
                 ++partition_index) {
               const auto& part = partition_descriptor.getPartitions()[partition_index];
               for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size();
                    ++chunk_index) {
                  SPDLOG_DEBUG(
                     "build - building sequence store for amino acid "
                     "sequence {} and partition {}",
                     aa_name,
                     partition_index
                  );

                  silo::ZstdFastaTableReader sequence_input(
                     preprocessing_db.getConnection(),
                     "gene_" + aa_name,
                     reference_sequence,
                     "sequence",
                     fmt::format("partition_id = {}", partition_index),
                     order_by_clause
                  );
                  database.partitions.at(partition_index)
                     .aa_sequences.at(aa_name)
                     .fill(sequence_input);
               }
            }
         }
      );
      SPDLOG_INFO("build - finished amino acid sequence {}", aa_name);
   }
}

}  // namespace silo::preprocessing

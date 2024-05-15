#include "silo/preprocessing/preprocessor.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/block_timer.h"
#include "silo/common/fasta_reader.h"
#include "silo/common/string_utils.h"
#include "silo/common/table_reader.h"
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
#include "silo/zstdfasta/zstdfasta_table_reader.h"

namespace silo::preprocessing {

constexpr std::string_view FASTA_EXTENSION = ".fasta";
const std::string INSERTIONS_TABLE_NAME_SUFFIX = "insertions";

namespace {
template <typename SymbolType>
std::string getInsertionsTableName() {
   return fmt::format("{}{}", SymbolType::PREFIX, INSERTIONS_TABLE_NAME_SUFFIX);
}

}  // namespace

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
      alias_lookup_(std::move(alias_lookup)) {
   nuc_sequences = reference_genomes_.getSequenceNames<Nucleotide>();
   aa_sequences = reference_genomes_.getSequenceNames<AminoAcid>();
   if (database_config.schema.date_to_sort_by.has_value()) {
      order_by_fields.emplace_back(database_config.schema.date_to_sort_by.value());
   }
   order_by_fields.emplace_back(database_config.schema.primary_key);

   prefixed_order_by_fields = prepend("order_by_field_", order_by_fields);
   prefixed_nuc_sequences = prepend(Nucleotide::PREFIX, nuc_sequences);
   prefixed_aa_sequences = prepend(AminoAcid::PREFIX, aa_sequences);
   prefixed_nuc_insertions_fields = prepend("nuc_insertions_", nuc_sequences);
   prefixed_aa_insertions_fields = prepend("aa_insertions_", aa_sequences);
}

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
      SPDLOG_DEBUG("preprocessing - creating insertions tables for building SILO");
      createInsertionsTableFromFile(
         nuc_sequences,
         preprocessing_config.getNucleotideInsertionsFilename(),
         getInsertionsTableName<Nucleotide>()
      );
      createInsertionsTableFromFile(
         aa_sequences,
         preprocessing_config.getAminoAcidInsertionsFilename(),
         getInsertionsTableName<AminoAcid>()
      );
      SPDLOG_DEBUG("preprocessing - creating partitioned sequence tables for building SILO");
      createPartitionedSequenceTablesFromSequenceFiles();
   }
   SPDLOG_INFO("preprocessing - finished initial loading of data");

   const auto partition_descriptor = preprocessing_db.getPartitionDescriptor();

   SPDLOG_INFO("preprocessing - building database");
   preprocessing_db.refreshConnection();
   return buildDatabase(
      partition_descriptor, preprocessing_config.getIntermediateResultsDirectory()
   );
}

void Preprocessor::buildTablesFromNdjsonInput(const std::filesystem::path& file_name) {
   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE metadata_table({});",
      boost::join(MetadataInfo::getMetadataSQLTypes(database_config), ",")
   ));

   if (!std::filesystem::exists(file_name)) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The specified input file {} does not exist.", file_name.string())
      );
   }

   if (std::filesystem::is_empty(file_name)) {
      SPDLOG_WARN(
         "The specified input file {} is empty. Ignoring its content.", file_name.string()
      );
      return;
   }

   SPDLOG_DEBUG("build - validating metadata file '{}' with config", file_name.string());
   MetadataInfo::validateNdjsonFile(file_name, database_config);

   (void)preprocessing_db.query(fmt::format(
      "INSERT INTO metadata_table BY NAME (SELECT {} FROM read_json_auto('{}'));",
      boost::join(MetadataInfo::getMetadataSelects(database_config), ","),
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
   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE metadata_table({});",
      boost::join(MetadataInfo::getMetadataSQLTypes(database_config), ",")
   ));

   MetadataInfo::validateMetadataFile(metadata_filename, database_config);

   (void)preprocessing_db.query(fmt::format(
      "INSERT INTO metadata_table BY NAME (SELECT {} FROM read_csv_auto('{}', delim = '\t', "
      "header = true));",
      boost::join(MetadataInfo::getMetadataFields(database_config), ","),
      metadata_filename.string()
   ));
}

std::string Preprocessor::makeNonNullKey(const std::string& field) {
   return fmt::format(R"(CASE WHEN {0} IS NULL THEN 'NULL'::VARCHAR ELSE '_' || {0} END)", field);
}

std::string Preprocessor::getPartitionKeySelect() const {
   if (database_config.schema.partition_by.has_value()) {
      if (preprocessing_config.getNdjsonInputFilename().has_value()) {
         return makeNonNullKey(fmt::format("metadata.\"{}\"", *database_config.schema.partition_by)
         );
      }
      return makeNonNullKey(fmt::format("\"{}\"", *database_config.schema.partition_by));
   }
   return "'NULL'::VARCHAR";
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
      makeNonNullKey(partition_by_field)
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
SELECT partitioning.partition_id AS partition_id, {0} as partition_key, metadata_table.*
FROM partition_keys,
     partitioning,
     metadata_table
WHERE ('_' || metadata_table.{1} = partition_keys.partition_key OR (metadata_table.{1} IS NULL
AND partition_keys.partition_key = 'NULL'))
  AND partition_keys.id >= partitioning.from_id
  AND partition_keys.id <= partitioning.to_id;
)-",
      makeNonNullKey(partition_by_field),
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
      "SELECT 'NULL'::VARCHAR AS partition_key, 0::bigint AS partition_id;"
   );

   (void)preprocessing_db.query(
      "CREATE OR REPLACE VIEW partitioned_metadata AS\n"
      "SELECT 0::bigint AS partition_id, metadata_table.*\n"
      "FROM metadata_table;"
   );
}

void Preprocessor::createPartitionedSequenceTablesFromNdjson(const std::filesystem::path& file_name
) {
   SequenceInfo::validateNdjsonFile(
      reference_genomes_, preprocessing_db.getConnection(), file_name
   );

   createUnalignedPartitionedSequenceFiles(file_name);

   createAlignedPartitionedSequenceViews(file_name);
}

void Preprocessor::createAlignedPartitionedSequenceViews(const std::filesystem::path& file_name) {
   std::string file_reader_sql;
   if (std::filesystem::is_empty(file_name)) {
      file_reader_sql = fmt::format(
         "SELECT ''::VARCHAR AS key, 'NULL'::VARCHAR AS partition_key, {}, {}, {}, {}, {} LIMIT 0",
         boost::join(silo::prepend("''::VARCHAR AS ", prefixed_nuc_sequences), ", "),
         boost::join(silo::prepend("''::VARCHAR AS ", prefixed_aa_sequences), ", "),
         boost::join(silo::prepend("''::VARCHAR AS ", prefixed_nuc_insertions_fields), ", "),
         boost::join(silo::prepend("''::VARCHAR AS ", prefixed_aa_insertions_fields), ", "),
         boost::join(silo::prepend("''::VARCHAR AS ", prefixed_order_by_fields), ", ")
      );
   } else {
      file_reader_sql = fmt::format(
         "SELECT metadata.\"{}\" AS key, {} AS partition_key, {}, {}, {}, {}, {} FROM "
         "read_json_auto('{}')",
         database_config.schema.primary_key,
         getPartitionKeySelect(),
         boost::join(
            silo::tie(
               "alignedNucleotideSequences.\"", nuc_sequences, "\" AS ", prefixed_nuc_sequences, ""
            ),
            ", "
         ),
         boost::join(
            silo::tie(
               "alignedAminoAcidSequences.\"", aa_sequences, "\" AS ", prefixed_aa_sequences, ""
            ),
            ", "
         ),
         boost::join(
            silo::tie(
               "nucleotideInsertions.\"",
               nuc_sequences,
               "\" AS ",
               prefixed_nuc_insertions_fields,
               ""
            ),
            ", "
         ),
         boost::join(
            silo::tie(
               "aminoAcidInsertions.\"", aa_sequences, "\" AS ", prefixed_aa_insertions_fields, ""
            ),
            ", "
         ),
         boost::join(
            silo::tie("metadata.\"", order_by_fields, "\" AS ", prefixed_order_by_fields, ""), ", "
         ),
         file_name.string()
      );
   }

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE sequence_table AS\n"
      "SELECT key, partition_key_to_partition.partition_id AS partition_id, {}, {}, {}, {} \n"
      "FROM ({}) file_reader "
      "JOIN partition_key_to_partition "
      "ON (file_reader.partition_key = partition_key_to_partition.partition_key);",
      boost::join(
         SequenceInfo::getAlignedSequenceSelects(reference_genomes_, preprocessing_db), ", "
      ),
      boost::join(prefixed_nuc_insertions_fields, ", "),
      boost::join(prefixed_aa_insertions_fields, ", "),
      boost::join(prefixed_order_by_fields, ", "),
      file_reader_sql
   ));

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE VIEW {} AS\n"
      "SELECT key, partition_id, {}, {} \n"
      "FROM sequence_table;",
      getInsertionsTableName<Nucleotide>(),
      boost::join(
         silo::tie("", prefixed_nuc_insertions_fields, " AS \"", nuc_sequences, "\" "), ","
      ),
      boost::join(prefixed_order_by_fields, ",")
   ));

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE VIEW {} AS\n"
      "SELECT key, partition_id, {}, {} \n"
      "FROM sequence_table;",
      getInsertionsTableName<AminoAcid>(),
      boost::join(silo::tie("", prefixed_aa_insertions_fields, " AS \"", aa_sequences, "\" "), ","),
      boost::join(prefixed_order_by_fields, ",")
   ));

   for (const auto& prefixed_nuc_name : prefixed_nuc_sequences) {
      (void)preprocessing_db.query(fmt::format(
         "CREATE OR REPLACE VIEW {0} AS\n"
         "SELECT key, {0} AS sequence, partition_id, {1} "
         "FROM sequence_table;",
         prefixed_nuc_name,
         boost::join(prefixed_order_by_fields, ",")
      ));
   }

   for (const auto& prefixed_aa_name : prefixed_aa_sequences) {
      (void)preprocessing_db.query(fmt::format(
         "CREATE OR REPLACE VIEW {0} AS\n"
         "SELECT key, {0} AS sequence, partition_id, {1} "
         "FROM sequence_table;",
         prefixed_aa_name,
         boost::join(prefixed_order_by_fields, ",")
      ));
   }
}

void Preprocessor::createUnalignedPartitionedSequenceFiles(const std::filesystem::path& file_name) {
   for (const auto& [seq_name, _] : reference_genomes_.raw_nucleotide_sequences) {
      const std::string file_reader_sql =
         std::filesystem::is_empty(file_name)
            ? fmt::format(
                 "SELECT ''::VARCHAR AS key, 'NULL'::VARCHAR as partition_key,"
                 " ''::VARCHAR AS unaligned_nuc_{} LIMIT 0",
                 seq_name
              )
            : fmt::format(
                 "SELECT metadata.\"{0}\" AS key, {1} AS partition_key, "
                 "       unalignedNucleotideSequences.\"{2}\" AS unaligned_nuc_{2} "
                 "FROM read_json_auto('{3}')",
                 database_config.schema.primary_key,
                 getPartitionKeySelect(),
                 seq_name,
                 file_name.string()
              );
      const std::string table_sql = fmt::format(
         "SELECT key, {}, partition_key_to_partition.partition_id \n"
         "FROM ({}) file_reader "
         "JOIN partition_key_to_partition "
         "ON (file_reader.partition_key = partition_key_to_partition.partition_key) ",
         SequenceInfo::getUnalignedSequenceSelect(seq_name, preprocessing_db),
         file_reader_sql
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

void Preprocessor::createInsertionsTableFromFile(
   const std::vector<std::string>& expected_sequences,
   const std::filesystem::path& insertion_file,
   const std::string& table_name
) {
   std::set<std::string> expected_sequence_columns;
   for (const auto& sequence_name : expected_sequences) {
      expected_sequence_columns.emplace(sequence_name);
   }

   auto result = preprocessing_db.query(fmt::format(
      "SELECT * FROM read_csv_auto('{}', "
      "delim = '\t', header = true) LIMIT 0;",
      insertion_file.string()
   ));

   std::vector<std::string> column_structs;
   for (size_t idx = 0; idx < result->ColumnCount(); idx++) {
      const std::string& actual_column_name = result->ColumnName(idx);
      if (actual_column_name == database_config.schema.primary_key) {
         column_structs.emplace_back(fmt::format("'{}': 'VARCHAR'", actual_column_name));
         continue;
      }
      if (expected_sequence_columns.contains(actual_column_name)) {
         column_structs.emplace_back(fmt::format("'{}': 'VARCHAR[]'", actual_column_name));
         continue;
      }
      const std::string error_message = fmt::format(
         "Column in '{}' ({}) is not equal to the primary key ({}) or a "
         "sequence name ({}).",
         insertion_file.string(),
         actual_column_name,
         database_config.schema.primary_key,
         boost::join(expected_sequence_columns, ",")
      );
      SPDLOG_ERROR(error_message);
      throw silo::preprocessing::PreprocessingException(error_message);
   }

   (void)preprocessing_db.query(fmt::format(
      R"-(
      CREATE OR REPLACE TABLE {0} AS
      (SELECT ins."{1}" AS key, {2}, {3}, partition_id
      FROM read_csv('{4}', delim = '\t', header = true,
                    columns = {{{5}}}) ins
      INNER JOIN partitioned_metadata ON ins."{1}" == partitioned_metadata."{1}" );)-",
      table_name,
      database_config.schema.primary_key,
      boost::join(
         silo::tie("ins.\"", expected_sequences, "\" AS \"", expected_sequences, "\" "), ","
      ),
      boost::join(
         silo::tie(
            "partitioned_metadata.\"", order_by_fields, "\" AS ", prefixed_order_by_fields, " "
         ),
         ","
      ),
      insertion_file.string(),
      boost::join(column_structs, ",")
   ));
}

void Preprocessor::createPartitionedSequenceTablesFromSequenceFiles() {
   for (const auto& [sequence_name, reference_sequence] :
        reference_genomes_.raw_nucleotide_sequences) {
      createPartitionedTableForSequence<Nucleotide>(
         sequence_name,
         reference_sequence,
         preprocessing_config.getNucFilenameNoExtension(sequence_name)
            .replace_extension(FASTA_EXTENSION)
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
      createPartitionedTableForSequence<AminoAcid>(
         sequence_name,
         reference_sequence,
         preprocessing_config.getGeneFilenameNoExtension(sequence_name)
            .replace_extension(FASTA_EXTENSION)
      );
   }
}

template <typename SymbolType>
void Preprocessor::createPartitionedTableForSequence(
   const std::string& sequence_name,
   const std::string& reference_sequence,
   const std::filesystem::path& filename
) {
   const std::string raw_table_name =
      fmt::format("{}{}{}", "raw_", SymbolType::PREFIX, sequence_name);
   const std::string table_name = fmt::format("{}{}", SymbolType::PREFIX, sequence_name);

   preprocessing_db.generateSequenceTableFromFasta(raw_table_name, reference_sequence, filename);

   (void)preprocessing_db.query(fmt::format(
      R"-(
         CREATE OR REPLACE VIEW {} AS
         SELECT key, sequence,
         partitioned_metadata.partition_id AS partition_id, {}
         FROM {} AS raw RIGHT JOIN partitioned_metadata
         ON raw.key = partitioned_metadata."{}";
      )-",
      table_name,
      boost::join(
         silo::tie(
            "partitioned_metadata.\"", order_by_fields, "\" AS ", prefixed_order_by_fields, " "
         ),
         ","
      ),
      raw_table_name,
      database_config.schema.primary_key
   ));
}

Database Preprocessor::buildDatabase(
   const preprocessing::Partitions& partition_descriptor,
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

         buildMetadataStore(
            database, partition_descriptor, "ORDER BY " + boost::join(order_by_fields, ",")
         );

         SPDLOG_INFO("build - finished metadata store");
      });

      tasks.run([&]() {
         const std::string order_by_clause =
            "ORDER BY " + boost::join(prefixed_order_by_fields, ",");
         SPDLOG_INFO("build - building nucleotide sequence stores");
         buildSequenceStore<Nucleotide>(database, partition_descriptor, order_by_clause);
         SPDLOG_INFO("build - finished nucleotide sequence stores");

         SPDLOG_INFO("build - building amino acid sequence stores");
         buildSequenceStore<AminoAcid>(database, partition_descriptor, order_by_clause);
         SPDLOG_INFO("build - finished amino acid sequence stores");
      });

      tasks.wait();
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

template <typename SymbolType>
void Preprocessor::buildSequenceStore(
   Database& database,
   const preprocessing::Partitions& partition_descriptor,
   const std::string& order_by_clause
) {
   for (const auto& pair : reference_genomes_.getRawSequenceMap<SymbolType>()) {
      const std::string& sequence_name = pair.first;
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
                     "build - building aligned sequence store for {} "
                     "sequence {} and partition {}",
                     SymbolType::SYMBOL_NAME,
                     sequence_name,
                     partition_index
                  );

                  auto& sequence_store = database.partitions.at(partition_index)
                                            .template getSequenceStores<SymbolType>()
                                            .at(sequence_name);

                  silo::ZstdFastaTableReader sequence_input(
                     preprocessing_db.getConnection(),
                     fmt::format("{}{}", SymbolType::PREFIX, sequence_name),
                     reference_sequence,
                     "sequence",
                     fmt::format("partition_id = {}", partition_index),
                     order_by_clause
                  );
                  sequence_store.fill(sequence_input);

                  const silo::ColumnFunction column_function{
                     sequence_name,
                     [&sequence_store](size_t row_id, const duckdb::Value& value) {
                        if (value.IsNull()) {
                           return;
                        }
                        for (const auto& child : duckdb::ListValue::GetChildren(value)) {
                           sequence_store.insertInsertion(row_id, child.GetValue<std::string>());
                        }
                     }
                  };
                  silo::TableReader(
                     preprocessing_db.getConnection(),
                     getInsertionsTableName<SymbolType>(),
                     "key",
                     {column_function},
                     fmt::format("partition_id = {}", partition_index),
                     order_by_clause
                  )
                     .read();

                  sequence_store.finalize();
               }
            }
         }
      );
      SPDLOG_INFO("build - finished {} sequence {}", SymbolType::SYMBOL_NAME, sequence_name);
   }
}

}  // namespace silo::preprocessing

#include "silo/preprocessing/preprocessor.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_invoke.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/block_timer.h"
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
#include "silo/preprocessing/validated_ndjson_file.h"
#include "silo/sequence_file_reader/fasta_reader.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/unaligned_sequence_store.h"
#include "silo/zstd/zstd_decompressor.h"
#include "silo/zstd/zstd_table.h"

namespace {
const std::string FASTA_EXTENSION = ".fasta";
const std::string INSERTIONS_TABLE_NAME_SUFFIX = "insertions";
template <typename SymbolType>
silo::preprocessing::Identifier getInsertionsTableName() {
   return silo::preprocessing::Identifier{
      fmt::format("{}{}", SymbolType::PREFIX, INSERTIONS_TABLE_NAME_SUFFIX)
   };
}

std::vector<std::string> getOrderByFieldVector(const silo::config::DatabaseConfig& database_config
) {
   if (database_config.schema.date_to_sort_by.has_value()) {
      return {database_config.schema.date_to_sort_by.value(), database_config.schema.primary_key};
   }
   return {database_config.schema.primary_key};
}

}  // namespace

namespace silo::preprocessing {

Preprocessor::Preprocessor(
   config::PreprocessingConfig preprocessing_config_,
   config::DatabaseConfig database_config_,
   ReferenceGenomes reference_genomes_,
   PangoLineageAliasLookup alias_lookup
)
    : preprocessing_config(std::move(preprocessing_config_)),
      database_config(std::move(database_config_)),
      reference_genomes(std::move(reference_genomes_)),
      alias_lookup(std::move(alias_lookup)),
      preprocessing_db(preprocessing_config.getPreprocessingDatabaseLocation(), reference_genomes),
      nuc_sequence_identifiers_without_prefix(
         Identifiers{reference_genomes.getSequenceNames<Nucleotide>()}
      ),
      aa_sequence_identifiers_without_prefix(
         Identifiers{reference_genomes.getSequenceNames<AminoAcid>()}
      ),
      nuc_sequence_identifiers(nuc_sequence_identifiers_without_prefix.prefix("nuc_")),
      aa_sequence_identifiers(aa_sequence_identifiers_without_prefix.prefix("aa_")),
      unaligned_nuc_sequences(nuc_sequence_identifiers_without_prefix.prefix("unaligned_nuc_")),
      order_by_fields_without_prefix(Identifiers{getOrderByFieldVector(database_config)}),
      order_by_fields(order_by_fields_without_prefix.prefix("order_by_field_")),
      nuc_insertions_fields(
         Identifiers{reference_genomes.getSequenceNames<Nucleotide>()}.prefix("nuc_insertions_")
      ),
      aa_insertions_fields(
         Identifiers{reference_genomes.getSequenceNames<AminoAcid>()}.prefix("aa_insertions_")
      ) {}

Database Preprocessor::preprocess() {
   finalizeConfig();

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
      auto input_file = ValidatedNdjsonFile::validateFileAgainstConfig(
         ndjson_input_filename.value(), database_config, reference_genomes
      );
      SPDLOG_DEBUG(
         "preprocessing - building preprocessing tables from ndjson input '{}'",
         ndjson_input_filename.value().string()
      );
      buildTablesFromNdjsonInput(input_file);
      SPDLOG_DEBUG("preprocessing - building partitioning tables");
      buildPartitioningTable();
      SPDLOG_DEBUG("preprocessing - creating compressed sequence views for building SILO");
      createPartitionedSequenceTablesFromNdjson(input_file);
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
      createInsertionsTableFromFile<Nucleotide>(
         preprocessing_config.getNucleotideInsertionsFilename()
      );
      createInsertionsTableFromFile<AminoAcid>(preprocessing_config.getAminoAcidInsertionsFilename()
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

void Preprocessor::finalizeConfig() {
   const auto& nuc_sequence_names = reference_genomes.getSequenceNames<Nucleotide>();
   const auto& aa_sequence_names = reference_genomes.getSequenceNames<AminoAcid>();
   if (nuc_sequence_names.size() == 1 && !database_config.default_nucleotide_sequence.has_value()) {
      database_config.default_nucleotide_sequence = nuc_sequence_names.at(0);
   }
   if (aa_sequence_names.size() == 1 && !database_config.default_amino_acid_sequence.has_value()) {
      database_config.default_amino_acid_sequence = aa_sequence_names.at(0);
   }

   validateConfig();
}

void Preprocessor::validateConfig() {
   const auto& nuc_sequence_names = reference_genomes.getSequenceNames<Nucleotide>();
   const auto& aa_sequence_names = reference_genomes.getSequenceNames<AminoAcid>();
   const bool default_nucleotide_sequence_is_not_in_reference =
      database_config.default_nucleotide_sequence.has_value() &&
      std::ranges::find(nuc_sequence_names, *database_config.default_nucleotide_sequence) ==
         nuc_sequence_names.end();
   if (default_nucleotide_sequence_is_not_in_reference) {
      throw silo::preprocessing::PreprocessingException(
         "The default nucleotide sequence that is set in the database config is not contained in "
         "the reference genomes."
      );
   }
   const bool default_amino_acid_sequence_is_not_in_reference =
      database_config.default_amino_acid_sequence.has_value() &&
      std::ranges::find(aa_sequence_names, *database_config.default_amino_acid_sequence) ==
         aa_sequence_names.end();
   if (default_amino_acid_sequence_is_not_in_reference) {
      throw silo::preprocessing::PreprocessingException(
         "The default amino acid sequence that is set in the database config is not contained in "
         "the reference genomes."
      );
   }
}

void Preprocessor::buildTablesFromNdjsonInput(const ValidatedNdjsonFile& input_file) {
   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE metadata_table({});",
      boost::join(MetadataInfo::getMetadataSQLTypes(database_config), ",")
   ));

   if (input_file.isEmpty()) {
      SPDLOG_WARN(
         "The specified input file {} is empty. Ignoring its content.",
         input_file.getFileName().string()
      );
      return;
   }

   (void)preprocessing_db.query(fmt::format(
      "INSERT INTO metadata_table BY NAME (SELECT {} FROM read_json_auto('{}'));",
      boost::join(MetadataInfo::getMetadataSelects(database_config), ","),
      input_file.getFileName().string()
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
         input_file.getFileName().string()
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
         return makeNonNullKey(fmt::format(
            "metadata.{}", Identifier::escapeIdentifier(*database_config.schema.partition_by)
         ));
      }
      return makeNonNullKey(Identifier::escapeIdentifier(*database_config.schema.partition_by));
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

void Preprocessor::createPartitionedSequenceTablesFromNdjson(const ValidatedNdjsonFile& input_file
) {
   createUnalignedPartitionedSequenceFiles(input_file);

   createAlignedPartitionedSequenceViews(input_file);
}

void Preprocessor::createAlignedPartitionedSequenceViews(const ValidatedNdjsonFile& input_file) {
   std::string file_reader_sql;
   if (input_file.isEmpty()) {
      file_reader_sql = fmt::format(
         "SELECT ''::VARCHAR AS key, 'NULL'::VARCHAR AS partition_key {} {} {} {} {} LIMIT 0",
         boost::join(
            silo::prepend(
               ", ''::VARCHAR AS ", nuc_sequence_identifiers.getEscapedIdentifierStrings()
            ),
            ""
         ),
         boost::join(
            silo::prepend(
               ", ''::VARCHAR AS ", aa_sequence_identifiers.getEscapedIdentifierStrings()
            ),
            ""
         ),
         boost::join(
            silo::prepend(", ''::VARCHAR AS ", nuc_insertions_fields.getEscapedIdentifierStrings()),
            ""
         ),
         boost::join(
            silo::prepend(", ''::VARCHAR AS ", aa_insertions_fields.getEscapedIdentifierStrings()),
            ""
         ),
         boost::join(
            silo::prepend(", ''::VARCHAR AS ", order_by_fields.getEscapedIdentifierStrings()), ""
         )
      );
   } else {
      file_reader_sql = fmt::format(
         "SELECT metadata.{} AS key, {} AS partition_key {} {} {} {} {} FROM "
         "read_json_auto('{}')",
         Identifier::escapeIdentifier(database_config.schema.primary_key),
         getPartitionKeySelect(),
         silo::tieAsString(
            ", alignedNucleotideSequences.",
            nuc_sequence_identifiers_without_prefix.getEscapedIdentifierStrings(),
            " AS ",
            nuc_sequence_identifiers.getEscapedIdentifierStrings(),
            ""
         ),
         silo::tieAsString(
            ", alignedAminoAcidSequences.",
            aa_sequence_identifiers_without_prefix.getEscapedIdentifierStrings(),
            " AS ",
            aa_sequence_identifiers.getEscapedIdentifierStrings(),
            ""
         ),
         silo::tieAsString(
            ", nucleotideInsertions.",
            nuc_sequence_identifiers_without_prefix.getEscapedIdentifierStrings(),
            " AS ",
            nuc_insertions_fields.getEscapedIdentifierStrings(),
            ""
         ),
         silo::tieAsString(
            ", aminoAcidInsertions.",
            aa_sequence_identifiers_without_prefix.getEscapedIdentifierStrings(),
            " AS ",
            aa_insertions_fields.getEscapedIdentifierStrings(),
            ""
         ),
         silo::tieAsString(
            ", metadata.",
            order_by_fields_without_prefix.getEscapedIdentifierStrings(),
            " AS ",
            order_by_fields.getEscapedIdentifierStrings(),
            ""
         ),
         input_file.getFileName().string()
      );
   }

   std::string sequence_select_statements;
   for (size_t sequence_idx = 0; sequence_idx < nuc_sequence_identifiers.size(); ++sequence_idx) {
      const auto prefixed_seq_identifier =
         nuc_sequence_identifiers.getIdentifier(sequence_idx).escape();
      sequence_select_statements += fmt::format(
         ", {} as {}",
         preprocessing_db.compress_nucleotide_functions.at(sequence_idx)
            ->generateSqlStatement(prefixed_seq_identifier),
         prefixed_seq_identifier
      );
   }
   for (size_t sequence_idx = 0; sequence_idx < aa_sequence_identifiers.size(); ++sequence_idx) {
      const auto prefixed_seq_identifier =
         aa_sequence_identifiers.getIdentifier(sequence_idx).escape();
      sequence_select_statements += fmt::format(
         ", {} as {}",
         preprocessing_db.compress_amino_acid_functions.at(sequence_idx)
            ->generateSqlStatement(prefixed_seq_identifier),
         prefixed_seq_identifier
      );
   }

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE sequence_table AS\n"
      "SELECT key, partition_key_to_partition.partition_id AS partition_id {} {} {}, {} \n"
      "FROM ({}) file_reader "
      "JOIN partition_key_to_partition "
      "ON (file_reader.partition_key = partition_key_to_partition.partition_key);",
      sequence_select_statements,
      boost::join(silo::prepend(", ", nuc_insertions_fields.getEscapedIdentifierStrings()), ""),
      boost::join(silo::prepend(", ", aa_insertions_fields.getEscapedIdentifierStrings()), ""),
      boost::join(order_by_fields.getEscapedIdentifierStrings(), ","),
      file_reader_sql
   ));

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE VIEW {} AS\n"
      "SELECT key, partition_id {}, {} \n"
      "FROM sequence_table;",
      getInsertionsTableName<Nucleotide>().escape(),
      boost::join(silo::prepend(", ", nuc_insertions_fields.getEscapedIdentifierStrings()), ""),
      boost::join(order_by_fields.getEscapedIdentifierStrings(), ",")
   ));

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE VIEW {} AS\n"
      "SELECT key, partition_id {}, {} \n"
      "FROM sequence_table;",
      getInsertionsTableName<AminoAcid>().escape(),
      boost::join(silo::prepend(", ", aa_insertions_fields.getEscapedIdentifierStrings()), ""),
      boost::join(order_by_fields.getEscapedIdentifierStrings(), ",")
   ));

   for (const auto& prefixed_nuc_name : nuc_sequence_identifiers.getEscapedIdentifierStrings()) {
      (void)preprocessing_db.query(fmt::format(
         "CREATE OR REPLACE VIEW {0} AS \n"
         "SELECT key, struct_pack(\"offset\" := 0, sequence := {0}) AS read, partition_id, {1} "
         "FROM sequence_table;",
         prefixed_nuc_name,
         boost::join(order_by_fields.getEscapedIdentifierStrings(), ",")
      ));
   }

   for (const auto& prefixed_aa_name : aa_sequence_identifiers.getEscapedIdentifierStrings()) {
      (void)preprocessing_db.query(fmt::format(
         "CREATE OR REPLACE VIEW {0} AS\n"
         "SELECT key, struct_pack(\"offset\" := 0, sequence := {0}) AS read, partition_id, {1} "
         "FROM sequence_table;",
         prefixed_aa_name,
         boost::join(order_by_fields.getEscapedIdentifierStrings(), ",")
      ));
   }
}

void Preprocessor::createUnalignedPartitionedSequenceFiles(const ValidatedNdjsonFile& input_file) {
   for (size_t sequence_idx = 0; sequence_idx < unaligned_nuc_sequences.size(); ++sequence_idx) {
      const auto escaped_seq_name =
         Identifier{reference_genomes.getSequenceNames<Nucleotide>().at(sequence_idx)}.escape();
      const auto prefixed_seq_identifier =
         unaligned_nuc_sequences.getIdentifier(sequence_idx).escape();
      const std::string file_reader_sql =
         input_file.isEmpty() ? fmt::format(
                                   "SELECT ''::VARCHAR AS key, 'NULL'::VARCHAR as partition_key,"
                                   " ''::VARCHAR AS {} LIMIT 0",
                                   prefixed_seq_identifier
                                )
                              : fmt::format(
                                   "SELECT metadata.{0} AS key, {1} AS partition_key, "
                                   "       unalignedNucleotideSequences.{2} AS {3} "
                                   "FROM read_json_auto('{4}')",
                                   Identifier::escapeIdentifier(database_config.schema.primary_key),
                                   getPartitionKeySelect(),
                                   escaped_seq_name,
                                   prefixed_seq_identifier,
                                   input_file.getFileName().string()
                                );
      const std::string table_sql = fmt::format(
         "SELECT key, struct_pack(\"offset\" := 0, sequence := {0}) AS sequence, "
         "partition_key_to_partition.partition_id \n"
         "FROM ({1}) file_reader "
         "JOIN partition_key_to_partition "
         "ON (file_reader.partition_key = partition_key_to_partition.partition_key) ",
         preprocessing_db.compress_nucleotide_functions.at(sequence_idx)
            ->generateSqlStatement(prefixed_seq_identifier),
         file_reader_sql
      );
      createUnalignedPartitionedSequenceFile(sequence_idx, table_sql);
   }
}

void Preprocessor::createUnalignedPartitionedSequenceFile(
   size_t sequence_idx,
   const std::string& table_sql
) {
   const std::filesystem::path save_location =
      preprocessing_config.getIntermediateResultsDirectory() /
      fmt::format("unaligned_nuc_{}", sequence_idx);
   preprocessing_db.query(fmt::format(
      "COPY ({}) TO '{}' (FORMAT PARQUET, PARTITION_BY ({}), OVERWRITE_OR_IGNORE);",
      table_sql,
      save_location.string(),
      "partition_id"
   ));
   preprocessing_db.query("VACUUM;");
}

template <typename SymbolType>
void Preprocessor::createInsertionsTableFromFile(const std::filesystem::path& insertion_file) {
   std::set<std::string> expected_sequence_columns;
   for (const auto& sequence_name : reference_genomes.getSequenceNames<SymbolType>()) {
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
         boost::join(expected_sequence_columns, ", ")
      );
      SPDLOG_ERROR(error_message);
      throw silo::preprocessing::PreprocessingException(error_message);
   }

   const Identifiers sequence_names{reference_genomes.getSequenceNames<SymbolType>()};

   (void)preprocessing_db.query(fmt::format(
      R"-(
      CREATE OR REPLACE TABLE {0} AS
      (SELECT ins."{1}" AS key {2} {3}, partition_id
      FROM read_csv('{4}', delim = '\t', header = true,
                    columns = {{{5}}}) ins
      INNER JOIN partitioned_metadata ON ins."{1}" == partitioned_metadata."{1}" );)-",
      getInsertionsTableName<SymbolType>().escape(),
      database_config.schema.primary_key,
      silo::tieAsString(
         ", ins.",
         sequence_names.getEscapedIdentifierStrings(),
         " AS ",
         getInsertionsFields<SymbolType>().getEscapedIdentifierStrings(),
         " "
      ),
      silo::tieAsString(
         ", partitioned_metadata.",
         order_by_fields_without_prefix.getEscapedIdentifierStrings(),
         " AS ",
         order_by_fields.getEscapedIdentifierStrings(),
         " "
      ),
      insertion_file.string(),
      boost::join(column_structs, ",")
   ));
}

void Preprocessor::createPartitionedSequenceTablesFromSequenceFiles() {
   for (size_t sequence_idx = 0; sequence_idx < nuc_sequence_identifiers.size(); ++sequence_idx) {
      const auto& reference_genome = reference_genomes.raw_nucleotide_sequences.at(sequence_idx);
      createPartitionedTableForSequence<Nucleotide>(
         sequence_idx,
         nuc_sequence_identifiers.getIdentifier(sequence_idx),
         reference_genome,
         preprocessing_config.getNucFilenameNoExtension(sequence_idx)
            .replace_extension(FASTA_EXTENSION)
      );
   }

   for (size_t sequence_idx = 0; sequence_idx < nuc_sequence_identifiers.size(); ++sequence_idx) {
      preprocessing_db.generateSequenceTableViaFile(
         "unaligned_tmp",
         reference_genomes.raw_nucleotide_sequences.at(sequence_idx),
         preprocessing_config.getUnalignedNucFilenameNoExtension(sequence_idx)
      );
      createUnalignedPartitionedSequenceFile(
         sequence_idx,
         fmt::format(
            "SELECT partitioned_metadata.{0} AS key, unaligned_tmp.read AS sequence, "
            "partitioned_metadata.partition_id AS partition_id "
            "FROM unaligned_tmp RIGHT JOIN partitioned_metadata "
            "ON unaligned_tmp.key = partitioned_metadata.{0} ",
            Identifier::escapeIdentifier(database_config.schema.primary_key)
         )
      );
      preprocessing_db.query("DROP TABLE IF EXISTS unaligned_tmp;");
   }

   for (size_t sequence_idx = 0; sequence_idx < aa_sequence_identifiers.size(); ++sequence_idx) {
      const auto& reference_genome = reference_genomes.raw_aa_sequences.at(sequence_idx);
      createPartitionedTableForSequence<AminoAcid>(
         sequence_idx,
         aa_sequence_identifiers.getIdentifier(sequence_idx),
         reference_genome,
         preprocessing_config.getGeneFilenameNoExtension(sequence_idx)
            .replace_extension(FASTA_EXTENSION)
      );
   }
}

template <typename SymbolType>
void Preprocessor::createPartitionedTableForSequence(
   size_t sequence_idx,
   const Identifier& prefixed_sequence_identifier,
   const std::string& compression_dictionary,
   const std::filesystem::path& filename
) {
   const std::string raw_table_name = fmt::format("raw_{}{}", SymbolType::PREFIX, sequence_idx);

   preprocessing_db.generateSequenceTableViaFile(raw_table_name, compression_dictionary, filename);

   (void)preprocessing_db.query(fmt::format(
      R"-(
         CREATE OR REPLACE VIEW {} AS
         SELECT key, read,
         partitioned_metadata.partition_id AS partition_id {}
         FROM {} AS raw RIGHT JOIN partitioned_metadata
         ON raw.key = partitioned_metadata.{};
      )-",
      prefixed_sequence_identifier.escape(),
      silo::tieAsString(
         ", partitioned_metadata.",
         order_by_fields_without_prefix.getEscapedIdentifierStrings(),
         " AS ",
         order_by_fields.getEscapedIdentifierStrings(),
         " "
      ),
      raw_table_name,
      Identifier::escapeIdentifier(database_config.schema.primary_key)
   ));
}

Database Preprocessor::buildDatabase(
   const preprocessing::Partitions& partition_descriptor,
   const std::filesystem::path& intermediate_results_directory
) {
   Database database;
   database.database_config = database_config;
   database.alias_key = alias_lookup;
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
      database.initializeNucSequences(
         reference_genomes.nucleotide_sequence_names, reference_genomes.nucleotide_sequences
      );
      database.initializeAASequences(
         reference_genomes.aa_sequence_names, reference_genomes.aa_sequences
      );

      tbb::parallel_invoke(
         [&]() {
            SPDLOG_INFO("build - building metadata store in parallel");

            buildMetadataStore(
               database,
               partition_descriptor,
               "ORDER BY " +
                  boost::join(order_by_fields_without_prefix.getEscapedIdentifierStrings(), ",")
            );

            SPDLOG_INFO("build - finished metadata store");
         },
         [&]() {
            const std::string order_by_clause =
               "ORDER BY " + boost::join(order_by_fields.getEscapedIdentifierStrings(), ",");
            SPDLOG_INFO("build - building nucleotide sequence stores");
            buildSequenceStore<Nucleotide>(database, partition_descriptor, order_by_clause);
            SPDLOG_INFO("build - finished nucleotide sequence stores");

            SPDLOG_INFO("build - building amino acid sequence stores");
            buildSequenceStore<AminoAcid>(database, partition_descriptor, order_by_clause);
            SPDLOG_INFO("build - finished amino acid sequence stores");
         }
      );
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
      auto& column_group = database.partitions.at(partition_id).columns;
      std::vector<ColumnFunction> column_functions;
      column_functions.reserve(database_config.schema.metadata.size());
      for (auto& item : database_config.schema.metadata) {
         column_functions.emplace_back(
            Identifier{item.name},
            [&](size_t /*chunk_offset*/, const duckdb::Vector& vector, size_t chunk_size) {
               for (size_t row_in_chunk = 0; row_in_chunk < chunk_size; row_in_chunk++) {
                  const auto& value = vector.GetValue(row_in_chunk);
                  if (value.IsNull()) {
                     column_group.addNullToColumn(item.name, item.getColumnType());
                  } else {
                     column_group.addValueToColumn(item.name, item.getColumnType(), value);
                  }
               }
            }
         );
      }
      TableReader table_reader(
         preprocessing_db.getConnection(),
         Identifier{"partitioned_metadata"},
         Identifier{database_config.schema.primary_key},
         column_functions,
         fmt::format("partition_id = {}", partition_id),
         order_by_clause
      );

      int64_t fill_time;
      {
         const silo::common::BlockTimer timer(fill_time);
         const size_t number_of_rows = table_reader.read();
         database.partitions.at(partition_id).sequence_count += number_of_rows;
      }
      SPDLOG_DEBUG(
         "build - finished fill columns for partition {} in {} microseconds",
         partition_id,
         fill_time
      );
      SPDLOG_INFO("build - finished columns for partition {}", partition_id);
   }
}

namespace {
template <typename SymbolType>
ColumnFunction createInsertionsLambda(
   Identifier sequence_name,
   SequenceStorePartition<SymbolType>& sequence_store
) {
   return ColumnFunction{
      std::move(sequence_name),
      [&sequence_store](size_t chunk_offset, const duckdb::Vector& vector, size_t chunk_size) {
         for (size_t row_in_chunk = 0; row_in_chunk < chunk_size; row_in_chunk++) {
            const auto& value = vector.GetValue(row_in_chunk);
            if (value.IsNull()) {
               continue;
            }
            for (const auto& child : duckdb::ListValue::GetChildren(value)) {
               sequence_store.insertInsertion(
                  chunk_offset + row_in_chunk, child.GetValue<std::string>()
               );
            }
         }
      }
   };
}

}  // namespace

template <>
Identifiers Preprocessor::getInsertionsFields<Nucleotide>() {
   return nuc_insertions_fields;
}

template <>
Identifiers Preprocessor::getInsertionsFields<AminoAcid>() {
   return aa_insertions_fields;
}

template <>
Identifiers Preprocessor::getSequenceIdentifiers<Nucleotide>() {
   return nuc_sequence_identifiers;
}

template <>
Identifiers Preprocessor::getSequenceIdentifiers<AminoAcid>() {
   return aa_sequence_identifiers;
}

template <typename SymbolType>
ColumnFunction Preprocessor::createRawReadLambda(
   ZstdDecompressor& decompressor,
   silo::SequenceStorePartition<SymbolType>& sequence_store
) {
   return ColumnFunction{
      Identifier{"read"},
      [&decompressor,
       &sequence_store](size_t /*chunk_offset*/, const duckdb::Vector& vector, size_t chunk_size) {
         for (size_t row_in_chunk = 0; row_in_chunk < chunk_size; row_in_chunk++) {
            ReadSequence& target = sequence_store.appendNewSequenceRead();
            const auto& value = vector.GetValue(row_in_chunk);
            if (value.IsNull()) {
               continue;
            }
            const auto& children = duckdb::StructValue::GetChildren(value);
            if (children[1].IsNull()) {
               continue;
            }
            decompressor.decompress(children[1].GetValueUnsafe<std::string>(), target.sequence);
            target.offset = children[0].GetValue<uint32_t>();
            target.is_valid = true;
         }
      }
   };
}

template <typename SymbolType>
void Preprocessor::buildSequenceStore(
   Database& database,
   const preprocessing::Partitions& partition_descriptor,
   const std::string& order_by_clause
) {
   for (size_t sequence_idx = 0;
        sequence_idx < reference_genomes.getSequenceNames<SymbolType>().size();
        sequence_idx++) {
      const std::string sequence_name =
         reference_genomes.getSequenceNames<SymbolType>().at(sequence_idx);
      const std::string& reference_sequence =
         reference_genomes.getRawSequences<SymbolType>().at(sequence_idx);
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

                  SequenceStorePartition<SymbolType>& sequence_store =
                     database.partitions.at(partition_index)
                        .template getSequenceStores<SymbolType>()
                        .at(sequence_name);

                  ZstdDecompressor decompressor(reference_sequence);

                  auto column_function_reads = createRawReadLambda(decompressor, sequence_store);

                  silo::TableReader(
                     preprocessing_db.getConnection(),
                     getSequenceIdentifiers<SymbolType>().getIdentifier(sequence_idx),
                     Identifier{"key"},
                     {column_function_reads},
                     fmt::format("partition_id = {}", partition_index),
                     order_by_clause
                  )
                     .read();

                  const silo::ColumnFunction column_function = createInsertionsLambda<SymbolType>(
                     getInsertionsFields<SymbolType>().getIdentifier(sequence_idx), sequence_store
                  );
                  silo::TableReader(
                     preprocessing_db.getConnection(),
                     getInsertionsTableName<SymbolType>(),
                     Identifier{"key"},
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

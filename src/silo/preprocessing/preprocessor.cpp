#include "silo/preprocessing/preprocessor.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <silo/zstdfasta/zstdfasta_table_reader.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/block_timer.h"
#include "silo/common/fasta_reader.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/metadata_info.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sequence_info.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstdfasta/zstd_decompressor.h"
#include "silo/zstdfasta/zstdfasta_table.h"

namespace silo::preprocessing {

Preprocessor::Preprocessor(
   const preprocessing::PreprocessingConfig preprocessing_config,
   const config::DatabaseConfig database_config
)
    : preprocessing_config(std::move(preprocessing_config)),
      database_config(std::move(database_config)),
      preprocessing_db(preprocessing_config.getPreprocessingDatabaseLocation().value_or(":memory:")
      ) {}

Database Preprocessor::preprocess() {
   SPDLOG_INFO("preprocessing - reading reference genome");
   ReferenceGenomes reference_genomes =
      ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

   SPDLOG_INFO("preprocessing - building alias key");
   const auto pango_lineage_definition_filename =
      preprocessing_config.getPangoLineageDefinitionFilename();
   PangoLineageAliasLookup alias_key;
   if (pango_lineage_definition_filename.has_value()) {
      alias_key = PangoLineageAliasLookup::readFromFile(pango_lineage_definition_filename.value());
   }

   if (preprocessing_config.getNdjsonInputFilename().has_value()) {
      buildTablesFromNdjsonInput(
         preprocessing_config.getNdjsonInputFilename().value(), reference_genomes
      );
      buildPartitioningTable();
      createSequenceViews(reference_genomes);
   } else {
      buildMetadataTableFromFile(preprocessing_config.getMetadataInputFilename());
      buildPartitioningTable();
      createPartitionedSequenceTables(reference_genomes);
   }

   preprocessing::Partitions partition_descriptor = preprocessing_db.getPartitionDescriptor();

   std::string order_by_clause = database_config.schema.getStrictOrderByClause();
   SPDLOG_INFO("preprocessing - order by clause is {}", order_by_clause);

   SPDLOG_INFO("preprocessing - building database");

   return buildDatabase(partition_descriptor, reference_genomes, order_by_clause, alias_key);
}

void Preprocessor::buildTablesFromNdjsonInput(
   const std::filesystem::path& file_name,
   const ReferenceGenomes& reference_genomes
) {
   SPDLOG_DEBUG("preprocessing - ndjson pipeline chosen");
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

   SequenceInfo sequence_info(reference_genomes);
   sequence_info.validate(preprocessing_db.getConnection(), file_name);

   const auto metadata_info = MetadataInfo::validateFromNdjsonFile(file_name, database_config);

   PreprocessingDatabase::registerSequences(reference_genomes);

   (void)preprocessing_db.query(fmt::format(
      "CREATE OR REPLACE TABLE preprocessing_table AS SELECT {}, "
      "{} \n FROM '{}' WHERE metadata.{} is not null;",
      boost::join(metadata_info.getMetadataSelects(), ","),
      boost::join(sequence_info.getSequenceSelects(), ","),
      file_name.string(),
      database_config.schema.primary_key
   ));

   (void)preprocessing_db.query(fmt::format(
      "create or replace view metadata_table as\n"
      "select {}\n"
      "from preprocessing_table;",
      boost::join(metadata_info.getMetadataFields(), ",")
   ));
}

void Preprocessor::buildMetadataTableFromFile(const std::filesystem::path& metadata_filename) {
   SPDLOG_DEBUG("preprocessing - classic metadata file pipeline chosen");
   MetadataInfo metadata_info =
      MetadataInfo::validateFromMetadataFile(metadata_filename, database_config);

   (void)preprocessing_db.query(fmt::format(
      "create or replace table metadata_table as\n"
      "select {}\n"
      "from '{}';",
      boost::join(metadata_info.getMetadataSelects(), ","),
      metadata_filename.string()
   ));
}

void Preprocessor::buildPartitioningTable() {
   if (database_config.schema.partition_by.has_value()) {
      buildPartitioningTableByColumn(database_config.schema.partition_by.value());
   } else {
      buildEmptyPartitioning();
   }
}

void Preprocessor::buildPartitioningTableByColumn(const std::string& partition_by_field) {
   SPDLOG_INFO("preprocessing - calculating partitions");

   (void)preprocessing_db.query(fmt::format(
      R"-(
create
or replace table partition_keys as
select row_number() over () - 1 as id, partition_key, count
from (SELECT {} as partition_key, COUNT(*) as count
      FROM metadata_table
      GROUP BY partition_key
      ORDER BY partition_key);
)-",
      partition_by_field
   ));

   // create Recursive Hierarchical Partitioning By Partition Field
   (void)preprocessing_db.query(
      R"-(
create or replace table partitioning as
with recursive
          allowed_count(allowed_count) as (select sum(count) / 32 from partition_keys),
          grouped_partition_keys(from_id, to_id, count) as
              (select id, id, count
               from partition_keys
               where id = 0
               union all
               select case when l1.count <= allowed_count then l1.from_id else l2.id end,
                      l2.id,
                      case when l1.count <= allowed_count
                           then l1.count + l2.count
                           else l2.count end
               from grouped_partition_keys l1,
                    partition_keys l2,
                    allowed_count
where l1.to_id + 1 = l2.id)
select row_number() over () - 1 as partition_id, from_id, to_id, count
from (select from_id, max(to_id) as to_id, max(count) as count
      from grouped_partition_keys
      group by from_id)
)-"
   );

   (void)preprocessing_db.query(
      R"-(
create
or replace table partition_key_to_partition as
select partition_keys.partition_key as partition_key,
  partitioning.partition_id as partition_id
from partition_keys,
     partitioning
where partition_keys.id >= partitioning.from_id
  AND partition_keys.id <= partitioning.to_id;
)-"
   );

   (void)preprocessing_db.query(fmt::format(
      R"-(
create
or replace view partitioned_metadata as
select partitioning.partition_id as partition_id, metadata_table.*
from partition_keys,
     partitioning,
     metadata_table
where (metadata_table.{0} = partition_keys.partition_key or (metadata_table.{0} is null
and partition_keys.partition_key is null))
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
create or replace table partitioning as
select 0::bigint as partition_id, 0::bigint as from_id, 0::bigint as to_id, count(*) as count
from metadata_table;
)-"
   );

   (void)preprocessing_db.query(
      "create or replace table partition_key_to_partition as\n"
      "select 0::bigint as partition_key, 0::bigint as partition_id;"
   );

   (void)preprocessing_db.query(
      "create\n"
      "or replace view partitioned_metadata as\n"
      "select 0::bigint as partition_id, metadata_table.*\n"
      "from metadata_table;"
   );
}

void Preprocessor::createSequenceViews(const ReferenceGenomes& reference_genomes) {
   std::string order_by_select =
      ", " + database_config.schema.primary_key + " as " + database_config.schema.primary_key;
   if (database_config.schema.date_to_sort_by.has_value()) {
      order_by_select += ", " + database_config.schema.date_to_sort_by.value() + " as " +
                         database_config.schema.date_to_sort_by.value();
   }
   std::string partition_by_where, partition_by_select;
   if (database_config.schema.partition_by.has_value()) {
      partition_by_select = "partition_key_to_partition.partition_id as partition_id";
      partition_by_where = fmt::format(
         "where (preprocessing_table.{0} = partition_key_to_partition.partition_key) or "
         "(preprocessing_table.{0} is null and "
         "partition_key_to_partition.partition_key is null)",
         database_config.schema.partition_by.value()
      );
   } else {
      partition_by_select = "0 as partition_id";
      partition_by_where = "";
   }

   for (const auto& [seq_name, _] : reference_genomes.raw_nucleotide_sequences) {
      (void)preprocessing_db.query(fmt::format(
         "create or replace view nuc_{0} as\n"
         "select {1} as key, nuc_{0} as sequence,"
         "{2}"
         "{3} \n"
         "from preprocessing_table, partition_key_to_partition "
         "{4};",
         seq_name,
         database_config.schema.primary_key,
         partition_by_select,
         order_by_select,
         partition_by_where
      ));
   }

   for (const auto& [seq_name, _] : reference_genomes.raw_aa_sequences) {
      (void)preprocessing_db.query(fmt::format(
         "create or replace view gene_{0} as\n"
         "select {1} as key, gene_{0} as sequence, "
         "{2}\n"
         "{3} \n"
         "from preprocessing_table, partition_key_to_partition "
         "{4};",
         seq_name,
         database_config.schema.primary_key,
         partition_by_select,
         order_by_select,
         partition_by_where
      ));
   }
}

void Preprocessor::createPartitionedSequenceTables(const ReferenceGenomes& reference_genomes) {
   std::string order_by_select = ", raw.key as " + database_config.schema.primary_key;
   if (database_config.schema.date_to_sort_by.has_value()) {
      order_by_select += ", partitioned_metadata." +
                         database_config.schema.date_to_sort_by.value() + " as " +
                         database_config.schema.date_to_sort_by.value();
   }

   for (const auto& [seq_name, reference_sequence] : reference_genomes.raw_nucleotide_sequences) {
      const std::string raw_table_name = "raw_nuc_" + seq_name;
      const std::string table_name = "nuc_" + seq_name;
      preprocessing_db.generateNucSequenceTable(
         raw_table_name,
         reference_sequence,
         preprocessing_config.getNucFilenameNoExtension(seq_name).replace_extension(
            silo::preprocessing::FASTA_EXTENSION
         )
      );

      (void)preprocessing_db.query(fmt::format(
         R"-(
create or replace view {} as
select key, sequence,
partitioned_metadata.partition_id as partition_id
{}
from {} raw, partitioned_metadata
where raw.key = partitioned_metadata.{};)-",
         table_name,
         order_by_select,
         raw_table_name,
         database_config.schema.primary_key
      ));
   }

   for (const auto& [seq_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
      const std::string raw_table_name = "raw_gene_" + seq_name;
      const std::string table_name = "gene_" + seq_name;
      preprocessing_db.generateNucSequenceTable(
         raw_table_name,
         reference_sequence,
         preprocessing_config.getGeneFilenameNoExtension(seq_name).replace_extension(
            silo::preprocessing::FASTA_EXTENSION
         )
      );

      (void)preprocessing_db.query(fmt::format(
         R"-(
create or replace view {} as
select key, sequence,
partitioned_metadata.partition_id as partition_id
{}
from {} raw, partitioned_metadata
where raw.key = partitioned_metadata.{};
)-",
         table_name,
         order_by_select,
         raw_table_name,
         database_config.schema.primary_key
      ));
   }
}

Database Preprocessor::buildDatabase(
   const preprocessing::Partitions& partition_descriptor,
   const ReferenceGenomes& reference_genomes,
   const std::string& order_by_clause,
   const silo::PangoLineageAliasLookup& alias_key
) {
   Database database;
   database.database_config = database_config;
   database.alias_key = alias_key;
   const DataVersion& data_version = DataVersion::mineDataVersion();
   SPDLOG_INFO("preprocessing - mining data data_version: {}", data_version.toString());
   database.setDataVersion(data_version);

   int64_t micros = 0;
   {
      const BlockTimer timer(micros);
      for (const auto& partition : partition_descriptor.getPartitions()) {
         database.partitions.emplace_back(partition.getPartitionChunks());
      }
      database.initializeColumns();
      database.initializeNucSequences(reference_genomes.nucleotide_sequences);
      database.initializeAASequences(reference_genomes.aa_sequences);

      SPDLOG_INFO("build - building metadata store");

      for (size_t partition_id = 0; partition_id < partition_descriptor.getPartitions().size();
           ++partition_id) {
         const auto& part = partition_descriptor.getPartitions()[partition_id];
         for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size();
              ++chunk_index) {
            uint32_t sequences_added = database.partitions[partition_id].columns.fill(
               preprocessing_db.getConnection(), partition_id, order_by_clause, database_config
            );
            database.partitions[partition_id].sequence_count += sequences_added;
         }
         SPDLOG_INFO("build - finished columns for partition {}", partition_id);
      }

      SPDLOG_INFO("build - building sequence stores");

      tbb::parallel_for(
         tbb::blocked_range<size_t>(0, partition_descriptor.getPartitions().size()),
         [&](const auto& local) {
            for (auto partition_index = local.begin(); partition_index != local.end();
                 ++partition_index) {
               const auto& part = partition_descriptor.getPartitions()[partition_index];
               for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size();
                    ++chunk_index) {
                  for (const auto& [nuc_name, reference_sequence] :
                       reference_genomes.raw_nucleotide_sequences) {
                     SPDLOG_DEBUG(
                        "build - building sequence store for nucleotide sequence {} and partition "
                        "{}",
                        nuc_name,
                        partition_index
                     );

                     silo::ZstdFastaTableReader sequence_input(
                        preprocessing_db.getConnection(),
                        "nuc_" + nuc_name,
                        reference_sequence,
                        fmt::format("partition_id = {}", partition_index),
                        order_by_clause
                     );
                     database.partitions[partition_index].nuc_sequences.at(nuc_name).fill(
                        sequence_input
                     );
                  }
                  for (const auto& [aa_name, reference_sequence] :
                       reference_genomes.raw_aa_sequences) {
                     SPDLOG_DEBUG(
                        "build - building sequence store for amino acid sequence {} and partition "
                        "{}",
                        aa_name,
                        partition_index
                     );

                     silo::ZstdFastaTableReader sequence_input(
                        preprocessing_db.getConnection(),
                        "gene_" + aa_name,
                        reference_sequence,
                        fmt::format("partition_id = {}", partition_index),
                        order_by_clause
                     );
                     database.partitions[partition_index].aa_sequences.at(aa_name).fill(
                        sequence_input
                     );
                  }
               }
               database.partitions.at(partition_index).flipBitmaps();
               SPDLOG_INFO("build - finished sequences for partition {}", partition_index);
            }
         }
      );
      database.finalizeInsertionIndexes();
   }

   SPDLOG_INFO("Build took {} ms", micros);
   SPDLOG_INFO("database info: {}", database.getDatabaseInfo());

   database.validate();

   return database;
}

}  // namespace silo::preprocessing

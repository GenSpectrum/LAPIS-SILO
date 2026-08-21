#include "rhydb/initialize/initializer.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "rhydb/append/ndjson_line_reader.h"
#include "rhydb/append/table_inserter.h"
#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/nucleotide_symbols.h"
#include "rhydb/common/panic.h"
#include "rhydb/common/phylo_tree.h"
#include "rhydb/database.h"
#include "rhydb/initialize/initialize_exception.h"
#include "rhydb/initialize/lineage_relation_table.h"
#include "rhydb/storage/column/column_metadata.h"
#include "rhydb/storage/column/column_type_visitor.h"
#include "rhydb/storage/column/dictionary_encoded_column.h"
#include "rhydb/storage/column/string_column.h"
#include "rhydb/storage/column/zstd_compressed_string_column.h"
#include "rhydb/storage/reference_genomes.h"

namespace rhydb::initialize {

namespace {

/// The `type` values written into the built-in `reference_genomes` table for the two sequence
/// kinds. They carry the nucleotide/amino-acid distinction that the type-less table would otherwise
/// lose, and are read back in `createSchemaFromConfigFiles`.
const std::string NUCLEOTIDE_SEQUENCE_TYPE = "nucleotide_sequence";
const std::string AMINO_ACID_SEQUENCE_TYPE = "amino_acid_sequence";

/// The column type of an unaligned nucleotide sequence column, which is not a reference kind of its
/// own: such a column is backed by the nucleotide reference it compresses against.
const std::string ZSTD_COMPRESSED_STRING_TYPE = "zstd_compressed_string";

// TODO(#741) we prepend the unalignedSequence columns (which are using the type
// ZstdCompressedStringColumn) with 'unaligned_'. This should be cleaned up with a
// refactor and breaking change of the current input format.
const std::string UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX = "unaligned_";

}  // namespace

void Initializer::createTableInDatabase(
   schema::TableName table_name,
   const config::InitializationFiles& initialization_files,
   Database& database
) {
   EVOBENCH_SCOPE("Initializer", "initializeDatabase");
   std::map<std::filesystem::path, common::LineageTreeAndIdMap> lineage_trees;
   for (const auto& file_path : initialization_files.getLineageDefinitionFilepaths()) {
      lineage_trees[file_path] =
         common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(file_path);
   }

   common::PhyloTree phylo_tree_file;
   auto opt_path = initialization_files.getPhyloTreeFilepath();
   if (opt_path.has_value()) {
      phylo_tree_file = common::PhyloTree::fromFile(opt_path.value());
   }

   loadReferences(
      table_name,
      ReferenceGenomes::readFromFile(initialization_files.getReferenceGenomeFilepath()),
      initialization_files.without_unaligned_sequences,
      database
   );

   createTableInDatabase(
      std::move(table_name),
      config::DatabaseConfig::getValidatedConfigFromFile(
         initialization_files.getDatabaseConfigFilepath()
      ),
      lineage_trees,
      phylo_tree_file,
      database
   );
}

void Initializer::createTableInDatabase(
   schema::TableName table_name,
   const config::DatabaseConfig& database_config,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
   const common::PhyloTree& phylo_tree,
   Database& database
) {
   // The sequence columns come from the `reference_columns` rows declared for this table alone, so
   // a second table does not pick up the first one's sequence columns.
   auto table_schema = createSchemaFromConfigFiles(
      database_config, database.getColumnReferences(table_name.getName()), lineage_trees, phylo_tree
   );
   database.createTable(std::move(table_name), std::move(table_schema));

   // Materialize a companion lineage relation table for every column configured with
   // `lineageIndexType` 'table' or 'both'. The tree name is resolved against the loaded lineage
   // definitions the same way the column's in-memory index is.
   for (const auto& config_metadata : database_config.schema.metadata) {
      if (!config_metadata.generatesLineageTable()) {
         continue;
      }
      auto lineage_tree =
         findLineageTreeForName(lineage_trees, config_metadata.generate_lineage_index.value());
      if (!lineage_tree.has_value()) {
         auto keys =
            lineage_trees | std::views::keys |
            std::views::transform([](const std::filesystem::path& path) { return path.string(); });
         throw InitializeException(
            "Column '{}' has lineage tree '{}' configured, but did not find corresponding lineage "
            "tree in the provided lineageDefinitionFilenames: {}",
            config_metadata.name,
            config_metadata.generate_lineage_index.value(),
            fmt::join(keys, ",")
         );
      }
      createLineageRelationTable(config_metadata.name, lineage_tree.value(), database);
   }
}

void Initializer::loadReferences(
   const schema::TableName& table_name,
   const ReferenceGenomes& reference_genomes,
   bool without_unaligned_sequences,
   Database& database
) {
   std::vector<ReferenceEntry> entries;
   entries.reserve(
      reference_genomes.nucleotide_sequence_names.size() +
      reference_genomes.aa_sequence_names.size()
   );
   std::vector<ColumnReferenceEntry> column_references;
   const auto collect_entries = [&](
                                   const std::vector<std::string>& names,
                                   const std::vector<std::string>& sequences,
                                   const std::string& type
                                ) {
      for (size_t sequence_idx = 0; sequence_idx < names.size(); ++sequence_idx) {
         const auto& name = names.at(sequence_idx);
         entries.push_back({.name = name, .reference = sequences.at(sequence_idx), .type = type});
         // A reference read from `reference_genomes.json` backs the column of the same name; the
         // file states no other mapping.
         column_references.push_back(
            {.table_name = table_name.getName(),
             .column_name = name,
             .column_type = type,
             .reference_name = name}
         );
         if (type == NUCLEOTIDE_SEQUENCE_TYPE && !without_unaligned_sequences) {
            // The unaligned form of a nucleotide sequence is a separate column, but it compresses
            // against that same reference -- so it points at the one entry rather than duplicating
            // the sequence under a second name.
            column_references.push_back(
               {.table_name = table_name.getName(),
                .column_name = UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX + name,
                .column_type = ZSTD_COMPRESSED_STRING_TYPE,
                .reference_name = name}
            );
         }
      }
   };
   collect_entries(
      reference_genomes.nucleotide_sequence_names,
      reference_genomes.raw_nucleotide_sequences,
      NUCLEOTIDE_SEQUENCE_TYPE
   );
   collect_entries(
      reference_genomes.aa_sequence_names,
      reference_genomes.raw_aa_sequences,
      AMINO_ACID_SEQUENCE_TYPE
   );

   // `addReferences` owns the `reference_genomes` row shape and enforces that names stay unique,
   // throwing `schema::DuplicatePrimaryKeyException` on a clash (which `preprocessing` reports).
   // The references have to be stored before the mapping is declared: `addColumnReferences` checks
   // that every reference it points at is really there.
   database.addReferences(entries);
   database.addColumnReferences(column_references);
}

void Initializer::createLineageRelationTable(
   std::string_view column_name,
   const common::LineageTreeAndIdMap& lineage_tree,
   Database& database
) {
   const std::string table_name_string{column_name};
   const schema::TableName table_name{table_name_string};
   if (database.tables.contains(table_name)) {
      throw InitializeException(
         "Cannot create lineage relation table '{}': a table with that name already exists.",
         table_name_string
      );
   }

   const schema::ColumnIdentifier id_column{.name = "id", .type = schema::ColumnType::STRING};
   const schema::ColumnIdentifier lineage_column{
      .name = "lineage", .type = schema::ColumnType::STRING
   };
   // The direct parent of `lineage` (null for a root). The transitive ancestry is walked from
   // these edges at query time rather than materialized.
   const schema::ColumnIdentifier parent_column{
      .name = "parent", .type = schema::ColumnType::STRING
   };
   // True when `lineage` has more than one direct parent (an edge into a recombinant node).
   const schema::ColumnIdentifier is_recombinant_edge_column{
      .name = "is_recombinant_edge", .type = schema::ColumnType::BOOL
   };
   // For a recombinant `lineage`, the most-recent common ancestor of its parents; null
   // otherwise.
   const schema::ColumnIdentifier recombinant_clade_ancestor_column{
      .name = "recombinant_clade_ancestor", .type = schema::ColumnType::STRING
   };
   auto table_schema = std::make_shared<schema::TableSchema>();
   table_schema->column_metadata.emplace(
      id_column, std::make_shared<storage::column::StringColumnMetadata>(id_column.name)
   );
   table_schema->column_metadata.emplace(
      lineage_column, std::make_shared<storage::column::StringColumnMetadata>(lineage_column.name)
   );
   table_schema->column_metadata.emplace(
      parent_column, std::make_shared<storage::column::StringColumnMetadata>(parent_column.name)
   );
   table_schema->column_metadata.emplace(
      is_recombinant_edge_column,
      std::make_shared<storage::column::ColumnMetadata>(is_recombinant_edge_column.name)
   );
   table_schema->column_metadata.emplace(
      recombinant_clade_ancestor_column,
      std::make_shared<storage::column::StringColumnMetadata>(recombinant_clade_ancestor_column.name
      )
   );
   table_schema->primary_key = id_column;
   database.createTable(table_name, std::move(table_schema));

   const auto rows = buildLineageRelationRows(lineage_tree);
   std::string ndjson;
   size_t row_id = 0;
   for (const auto& row : rows) {
      const nlohmann::json line{
         {"id", std::to_string(row_id++)},
         {"lineage", row.lineage},
         {"parent", row.parent.has_value() ? nlohmann::json(*row.parent) : nlohmann::json(nullptr)},
         {"is_recombinant_edge", row.is_recombinant_edge},
         {"recombinant_clade_ancestor",
          row.recombinant_clade_ancestor.has_value()
             ? nlohmann::json(*row.recombinant_clade_ancestor)
             : nlohmann::json(nullptr)}
      };
      ndjson += line.dump();
      ndjson += '\n';
   }
   std::stringstream ndjson_stream{ndjson};
   rhydb::append::NdjsonLineReader ndjson_reader{ndjson_stream};
   rhydb::append::appendDataToTable(database.tables.at(table_name), ndjson_reader);
   SPDLOG_INFO("Built lineage relation table '{}' with {} rows", table_name_string, rows.size());
}

struct ColumnMetadataInitializer {
   template <storage::column::Column ColumnType>
   void operator()(
      std::shared_ptr<storage::column::ColumnMetadata>& metadata,
      const config::DatabaseMetadata& config_metadata,
      const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
      const common::PhyloTree& phylo_tree_file
   );
};

template <>
void ColumnMetadataInitializer::operator()<storage::column::DictionaryEncodedColumn>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
   const common::PhyloTree& /*phylo_tree_file*/
) {
   if (config_metadata.generatesLineageColumnIndex()) {
      auto lineage_tree_name = config_metadata.generate_lineage_index.value();
      auto lineage_tree = Initializer::findLineageTreeForName(lineage_trees, lineage_tree_name);
      if (not lineage_tree.has_value()) {
         auto keys =
            lineage_trees | std::views::keys |
            std::views::transform([](const std::filesystem::path& path) { return path.string(); });
         throw InitializeException(
            "Column '{}' has lineage tree '{}' configured, but did not find corresponding lineage "
            "tree in the provided lineageDefinitionFilenames: {}",
            config_metadata.name,
            config_metadata.generate_lineage_index.value(),
            fmt::join(keys, ",")
         );
      }
      metadata = std::make_shared<storage::column::DictionaryEncodedColumn::Metadata>(
         config_metadata.name, lineage_tree.value(), config_metadata.treat_unknown_lineages_as_null
      );
   } else {
      metadata =
         std::make_shared<storage::column::DictionaryEncodedColumn::Metadata>(config_metadata.name);
   }
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::StringColumn>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& /*lineage_trees*/,
   const common::PhyloTree& phylo_tree_file
) {
   if (config_metadata.phylo_tree_node_identifier) {
      metadata = std::make_shared<storage::column::StringColumn::Metadata>(
         config_metadata.name, phylo_tree_file
      );
   } else {
      metadata = std::make_shared<storage::column::StringColumn::Metadata>(config_metadata.name);
   }
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::ZstdCompressedStringColumn>(
   std::shared_ptr<storage::column::ColumnMetadata>& /*metadata*/,
   const config::DatabaseMetadata& /*config_metadata*/,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& /*lineage_trees*/,
   const common::PhyloTree& /*phylo_tree_file*/
) {
   SILO_PANIC("unaligned nucleotide sequences cannot be in config::DatabaseMetadata");
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::SequenceColumn<Nucleotide>>(
   std::shared_ptr<storage::column::ColumnMetadata>& /*metadata*/,
   const config::DatabaseMetadata& /*config_metadata*/,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& /*lineage_trees*/,
   const common::PhyloTree& /*phylo_tree_file*/
) {
   SILO_PANIC("nucleotides cannot be in config::DatabaseMetadata");
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::SequenceColumn<AminoAcid>>(
   std::shared_ptr<storage::column::ColumnMetadata>& /*metadata*/,
   const config::DatabaseMetadata& /*config_metadata*/,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& /*lineage_trees*/,
   const common::PhyloTree& /*phylo_tree_file*/
) {
   SILO_PANIC("amino acid cannot be in config::DatabaseMetadata");
}

template <storage::column::Column ColumnType>
void ColumnMetadataInitializer::operator()(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& /*lineage_trees*/,
   const common::PhyloTree& /*phylo_tree_file*/
) {
   metadata = std::make_shared<typename ColumnType::Metadata>(config_metadata.name);
}

namespace {

void assertPrimaryKeyInMetadata(const rhydb::config::DatabaseConfig& database_config) {
   auto primary_key_metadata = std::ranges::find_if(
      database_config.schema.metadata,
      [&database_config](const auto& metadata) {
         return database_config.schema.primary_key == metadata.name;
      }
   );
   if (primary_key_metadata == database_config.schema.metadata.end()) {
      throw InitializeException("The primary key is not contained in the metadata.");
   }
}

void assertPrimaryKeyOfTypeString(const rhydb::config::DatabaseConfig& database_config) {
   auto primary_key_metadata = std::ranges::find_if(
      database_config.schema.metadata,
      [&database_config](const auto& metadata) {
         return database_config.schema.primary_key == metadata.name;
      }
   );
   auto primary_key_type = primary_key_metadata->getColumnType();
   if (primary_key_type != schema::ColumnType::STRING) {
      throw InitializeException(
         "The primary key must be of type STRING but it is of type {}",
         columnTypeToString(primary_key_type)
      );
   }
}

}  // namespace

std::shared_ptr<schema::TableSchema> Initializer::createSchemaFromConfigFiles(
   const config::DatabaseConfig& database_config,
   const std::vector<ResolvedColumnReference>& column_references,
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
   const common::PhyloTree& phylo_tree_file
) {
   assertPrimaryKeyInMetadata(database_config);
   assertPrimaryKeyOfTypeString(database_config);

   const schema::ColumnIdentifier primary_key{
      .name = database_config.schema.primary_key, .type = schema::ColumnType::STRING
   };

   std::map<schema::ColumnIdentifier, std::shared_ptr<storage::column::ColumnMetadata>>
      column_metadata;
   for (const auto& config_metadata : database_config.schema.metadata) {
      const schema::ColumnIdentifier column_identifier{
         .name = config_metadata.name, .type = config_metadata.getColumnType()
      };
      std::shared_ptr<storage::column::ColumnMetadata> metadata;
      storage::column::visit(
         column_identifier.type,
         ColumnMetadataInitializer{},
         metadata,
         config_metadata,
         lineage_trees,
         phylo_tree_file
      );
      column_metadata.emplace(column_identifier, metadata);
   }

   // The sequence columns come from the `reference_columns` declarations for this table: each row
   // already names the column, its type and the reference behind it, so nothing has to be inferred
   // from a reference's name here. Two rows may name the same reference -- an aligned nucleotide
   // column and its `unaligned_` companion do exactly that.
   for (const auto& column_reference : column_references) {
      const schema::ColumnIdentifier column_identifier{
         .name = column_reference.column_name, .type = column_reference.column_type
      };
      switch (column_reference.column_type) {
         case schema::ColumnType::NUCLEOTIDE_SEQUENCE:
            column_metadata.emplace(
               column_identifier,
               std::make_shared<storage::column::SequenceColumnMetadata<Nucleotide>>(
                  column_reference.column_name,
                  ReferenceGenomes::stringToVector<Nucleotide>(column_reference.reference)
               )
            );
            break;
         case schema::ColumnType::AMINO_ACID_SEQUENCE:
            column_metadata.emplace(
               column_identifier,
               std::make_shared<storage::column::SequenceColumnMetadata<AminoAcid>>(
                  column_reference.column_name,
                  ReferenceGenomes::stringToVector<AminoAcid>(column_reference.reference)
               )
            );
            break;
         case schema::ColumnType::ZSTD_COMPRESSED_STRING:
            // Named after the reference rather than the column, matching what the column metadata
            // has always been constructed with here.
            column_metadata.emplace(
               column_identifier,
               std::make_shared<storage::column::ZstdCompressedStringColumnMetadata>(
                  column_reference.reference_name, column_reference.reference
               )
            );
            break;
         default:
            throw InitializeException(
               "The '{}' table declares the column '{}' with type '{}', which takes no reference.",
               Database::REFERENCE_COLUMNS_TABLE_NAME,
               column_reference.column_name,
               columnTypeToString(column_reference.column_type)
            );
      }
   }

   return std::make_shared<schema::TableSchema>(column_metadata, primary_key);
}

std::optional<common::LineageTreeAndIdMap> Initializer::findLineageTreeForName(
   const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
   const std::string& lineage_tree_name
) {
   for (const auto& [path, lineage_tree] : lineage_trees) {
      if (path.filename() == lineage_tree_name || path.filename() == lineage_tree_name + ".yaml") {
         return lineage_tree;
      }
   }
   return std::nullopt;
}

}  // namespace rhydb::initialize

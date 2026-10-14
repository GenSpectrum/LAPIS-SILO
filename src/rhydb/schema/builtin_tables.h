#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "rhydb/schema/database_schema.h"

namespace rhydb::schema {

/// The name of the built-in table holding the reference genomes, one row per nucleotide and amino
/// acid sequence.
inline constexpr std::string_view REFERENCE_GENOMES_TABLE_NAME = "reference_genomes";

/// The values of the `type` column of the reference genomes table.
inline constexpr std::string_view REFERENCE_GENOMES_NUCLEOTIDE_TYPE = "nucleotide";
inline constexpr std::string_view REFERENCE_GENOMES_AMINO_ACID_TYPE = "amino_acid";

/// The name of the built-in table holding the lineage definition files, one row per column with a
/// lineage definition.
inline constexpr std::string_view LINEAGE_DEFINITIONS_TABLE_NAME = "lineage_definitions";

/// The columns of the lineage definitions table: the name of the lineage column (its primary key),
/// and the lineage definition file of that column, verbatim.
inline constexpr std::string_view LINEAGE_DEFINITIONS_COLUMN_NAME_COLUMN = "column_name";
inline constexpr std::string_view LINEAGE_DEFINITIONS_DEFINITION_COLUMN = "definition";

/// Built-in tables are ordinary tables (persisted, queryable and writable like any other) that
/// every database is guaranteed to contain. They are created empty together with the database, and
/// are added to databases loaded from a state that predates them.
[[nodiscard]] std::map<TableName, std::shared_ptr<TableSchema>> getBuiltinTableSchemas();

}  // namespace rhydb::schema

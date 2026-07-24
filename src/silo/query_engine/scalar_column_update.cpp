#include "silo/query_engine/scalar_column_update.h"

#include <optional>

#include <fmt/format.h>

#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/saneql/ast.h"
#include "silo/query_engine/saneql/parser.h"
#include "silo/storage/column_group.h"

namespace silo::query_engine {

void assignScalarLiteralToColumn(
   storage::ColumnGroup& columns,
   const schema::ColumnIdentifier& column,
   const std::string& value,
   const roaring::Roaring& row_ids
) {
   namespace ast = saneql::ast;

   // Parse the new value once, going through the same lexer/parser and literal extractors as
   // queries, so no type-specific string parsing is duplicated. A SaneQL `null` literal clears the
   // matched rows; every other literal must match the column's type.
   const auto literal = saneql::Parser{value}.parse();
   const bool is_null = ast::isNullLiteral(*literal);

   switch (column.type) {
      case schema::ColumnType::INT32:
         columns.int_columns.at(column.name)
            .update(
               row_ids, is_null ? std::nullopt : std::optional{ast::extractInt32Literal(*literal)}
            );
         return;
      case schema::ColumnType::FLOAT:
         columns.float_columns.at(column.name)
            .update(
               row_ids,
               is_null ? std::nullopt : std::optional{ast::extractNumericAsFloatLiteral(*literal)}
            );
         return;
      case schema::ColumnType::DATE32:
         columns.date32_columns.at(column.name)
            .update(row_ids, ast::extractOptionalDateValue(*literal));
         return;
      case schema::ColumnType::BOOL:
         columns.bool_columns.at(column.name)
            .update(
               row_ids, is_null ? std::nullopt : std::optional{ast::extractBoolLiteral(*literal)}
            );
         return;
      case schema::ColumnType::STRING: {
         auto& string_column = columns.string_columns.at(column.name);
         if (string_column.metadata->phylo_tree.has_value()) {
            throw IllegalQueryException(fmt::format(
               "Column '{}' is backed by a phylogenetic tree and cannot be updated, because it "
               "would break the tree's row bindings",
               column.name
            ));
         }
         string_column.update(
            row_ids, is_null ? std::nullopt : std::optional{ast::extractStringLiteral(*literal)}
         );
         return;
      }
      case schema::ColumnType::INDEXED_STRING: {
         auto& indexed_string_column = columns.indexed_string_columns.at(column.name);
         if (indexed_string_column.getLineageIndex().has_value()) {
            throw IllegalQueryException(fmt::format(
               "Column '{}' is backed by a lineage index and cannot be updated", column.name
            ));
         }
         indexed_string_column.update(
            row_ids, is_null ? std::nullopt : std::optional{ast::extractStringLiteral(*literal)}
         );
         return;
      }
      case schema::ColumnType::ZSTD_COMPRESSED_STRING:
         columns.zstd_compressed_string_columns.at(column.name)
            .update(
               row_ids, is_null ? std::nullopt : std::optional{ast::extractStringLiteral(*literal)}
            );
         return;
      default:
         throw IllegalQueryException(fmt::format(
            "Updating columns of type '{}' is not supported (column '{}')",
            schema::columnTypeToString(column.type),
            column.name
         ));
   }
}

}  // namespace silo::query_engine

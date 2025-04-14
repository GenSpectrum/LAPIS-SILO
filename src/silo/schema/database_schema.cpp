#include "silo/schema/database_schema.h"

#include "silo/storage/column/column_type_visitor.h"
#include "yaml-cpp/yaml.h"

namespace silo::schema {

bool isSequenceColumn(ColumnType type) {
   return type == ColumnType::NUCLEOTIDE_SEQUENCE || type == ColumnType::AMINO_ACID_SEQUENCE ||
          type == ColumnType::ZSTD_COMPRESSED_STRING;
}

std::optional<ColumnIdentifier> TableSchema::getColumn(std::string_view name) const {
   auto it = std::ranges::find_if(column_metadata, [&name](const auto& metadata_pair) {
      return metadata_pair.first.name == name;
   });
   if (it == column_metadata.end()) {
      return std::nullopt;
   }
   return it->first;
}

std::vector<ColumnIdentifier> TableSchema::getColumnIdentifiers() const {
   std::vector<ColumnIdentifier> result;
   for (const auto& [column_identifier, _] : column_metadata) {
      result.push_back(column_identifier);
   }
   return result;
}

template <>
std::optional<ColumnIdentifier> TableSchema::getDefaultSequenceName<Nucleotide>() const {
   return default_nucleotide_sequence;
}

template <>
std::optional<ColumnIdentifier> TableSchema::getDefaultSequenceName<AminoAcid>() const {
   return default_aa_sequence;
}

namespace {

ColumnType columnTypeFromString(const std::string& string) {
   if (string == "string") {
      return ColumnType::STRING;
   } else if (string == "indexedString") {
      return ColumnType::INDEXED_STRING;
   } else if (string == "date") {
      return ColumnType::DATE;
   } else if (string == "aminoAcidSequence") {
      return ColumnType::AMINO_ACID_SEQUENCE;
   } else if (string == "nucleotideSequence") {
      return ColumnType::NUCLEOTIDE_SEQUENCE;
   } else if (string == "float") {
      return ColumnType::FLOAT;
   } else if (string == "integer") {
      return ColumnType::INT;
   } else if (string == "bool") {
      return ColumnType::BOOL;
   } else if (string == "zstdCompressedString") {
      return ColumnType::ZSTD_COMPRESSED_STRING;
   }
   SILO_UNREACHABLE();
}

std::string columnTypeToString(ColumnType column_type) {
   switch (column_type) {
      case ColumnType::STRING:
         return "string";
      case ColumnType::INDEXED_STRING:
         return "indexedString";
      case ColumnType::DATE:
         return "date";
      case ColumnType::FLOAT:
         return "float";
      case ColumnType::INT:
         return "integer";
      case ColumnType::BOOL:
         return "bool";
      case ColumnType::NUCLEOTIDE_SEQUENCE:
         return "nucleotideSequence";
      case ColumnType::AMINO_ACID_SEQUENCE:
         return "aminoAcidSequence";
      case ColumnType::ZSTD_COMPRESSED_STRING:
         return "zstdCompressedString";
   }
   SILO_UNREACHABLE();
}
}  // namespace

class ColumnMetadataLoaderByType {
  public:
   template <typename ColumnType>
   std::shared_ptr<storage::column::ColumnMetadata> operator()(
      std::string column_name,
      const YAML::Node& node
   ) {
      return ColumnType::Metadata::fromYAML(column_name, node);
   }
};

TableSchema TableSchema::fromYAML(const YAML::Node& yaml_node) {
   std::map<ColumnIdentifier, std::shared_ptr<storage::column::ColumnMetadata>> column_metadata;
   for (const auto entry : yaml_node["columns"]) {
      std::string column_name = entry["name"].as<std::string>();
      ColumnType column_type = columnTypeFromString(entry["type"].as<std::string>());
      column_metadata.emplace(
         ColumnIdentifier{column_name, column_type},
         storage::column::visit(
            column_type, ColumnMetadataLoaderByType{}, column_name, entry["metadata"]
         )
      );
   }
   std::string primary_key_name = yaml_node["primaryKey"].as<std::string>();
   ColumnType primary_key_type =
      std::ranges::find_if(column_metadata, [&primary_key_name](const auto& key_and_metadata) {
         return primary_key_name == key_and_metadata.first.name;
      })->first.type;
   TableSchema schema{column_metadata, ColumnIdentifier{primary_key_name, primary_key_type}};
   if (yaml_node["defaultAASequence"].IsDefined()) {
      schema.default_aa_sequence = {
         yaml_node["defaultAASequence"].as<std::string>(), ColumnType::AMINO_ACID_SEQUENCE
      };
   }
   if (yaml_node["defaultNucleotideSequence"].IsDefined()) {
      schema.default_nucleotide_sequence = {
         yaml_node["defaultNucleotideSequence"].as<std::string>(), ColumnType::NUCLEOTIDE_SEQUENCE
      };
   }
   return schema;
}

YAML::Node TableSchema::toYAML() const {
   YAML::Node yaml_node;
   YAML::Node columns_node{YAML::NodeType::Sequence};
   for (const auto& [identifier, metadata] : column_metadata) {
      YAML::Node column_node{YAML::NodeType::Map};
      column_node["name"] = identifier.name;
      column_node["type"] = columnTypeToString(identifier.type);
      auto metadata_node = metadata->toYAML();
      if (metadata_node.IsDefined()) {
         column_node["metadata"] = metadata_node;
      }
      columns_node.push_back(std::move(column_node));
   }
   yaml_node["columns"] = std::move(columns_node);
   yaml_node["primaryKey"] = primary_key.name;
   if (default_aa_sequence.has_value()) {
      yaml_node["defaultAASequence"] = default_aa_sequence.value().name;
   }
   if (default_nucleotide_sequence.has_value()) {
      yaml_node["defaultNucleotideSequence"] = default_nucleotide_sequence.value().name;
   }
   return yaml_node;
}

TableName::TableName(std::string_view name) {
   for (char c : name) {
      if (c < 'a' || c > 'z') {
         throw std::runtime_error("Table names may only contain lower-case letters");
      }
   }
   this->name = name;
}

TableName default_table_name{"default"};

const TableName& TableName::getDefault() {
   return default_table_name;
}

const TableSchema& DatabaseSchema::getDefaultTableSchema() const {
   return tables.at(TableName::getDefault());
}

YAML::Node DatabaseSchema::toYAML() const {
   YAML::Node node;
   for (const auto& [table_name, table] : tables) {
      node[table_name.getName()] = table.toYAML();
   }
   return node;
}

DatabaseSchema DatabaseSchema::fromYAML(const YAML::Node& yaml) {
   std::map<TableName, TableSchema> tables;
   for (const auto& entry : yaml) {
      TableName table_name{entry.first.as<std::string>()};
      tables.emplace(table_name, TableSchema::fromYAML(entry.second));
   }
   return DatabaseSchema{.tables = tables};
}

}  // namespace silo::schema

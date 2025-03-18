#include "silo/storage/column/column_metadata.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

YAML::Node ColumnMetadata::toYAML() const {
   SPDLOG_INFO("Saving column metadata for column {}", column_name);
   return YAML::Node(YAML::NodeType::Undefined);
}

std::shared_ptr<ColumnMetadata> ColumnMetadata::fromYAML(
   std::string column_name,
   const YAML::Node& yaml
) {
   return std::make_shared<ColumnMetadata>(std::move(column_name));
}

}  // namespace silo::storage::column

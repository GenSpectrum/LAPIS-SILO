#include "silo/storage/column/column_metadata.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

YAML::Node CM::toYAML() {
   SPDLOG_INFO("Saving column metadata for column {}", column_name);
   return YAML::Node(YAML::NodeType::Undefined);
}

std::shared_ptr<CM> CM::fromYAML(std::string column_name, const YAML::Node& yaml) {
   return std::make_shared<CM>(std::move(column_name));
}

}  // namespace silo::storage::column

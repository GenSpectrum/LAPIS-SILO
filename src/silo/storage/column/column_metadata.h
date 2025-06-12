#pragma once

#include <string>

#include <yaml-cpp/yaml.h>

namespace silo::storage::column {

class ColumnMetadata {
  public:
   std::string column_name;

   explicit ColumnMetadata(std::string column_name)
       : column_name(column_name) {}

   virtual ~ColumnMetadata() = default;

   virtual YAML::Node toYAML() const;
   static std::shared_ptr<ColumnMetadata> fromYAML(std::string column_name, const YAML::Node& yaml);
};

}  // namespace silo::storage::column
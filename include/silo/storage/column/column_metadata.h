#pragma once

#include <string>

#include <yaml-cpp/yaml.h>

namespace silo::storage::column {

// TODO
class CM {
  public:
   std::string column_name;

   explicit CM(std::string column_name)
       : column_name(column_name) {}

   virtual ~CM() = default;

   virtual YAML::Node toYAML();
   static std::shared_ptr<CM> fromYAML(std::string column_name, const YAML::Node& yaml);
};

}  // namespace silo::storage::column
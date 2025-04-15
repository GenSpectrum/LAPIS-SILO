#pragma once

#include <filesystem>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "silo/common/lineage_name.h"

namespace silo::preprocessing {
using silo::common::LineageName;

class LineageDefinition {
  public:
   LineageName lineage_name;
   std::vector<LineageName> aliases;
   std::vector<LineageName> parents;
};

class LineageDefinitionFile {
  public:
   std::vector<LineageDefinition> lineages;
   std::string raw_file;

   static LineageDefinitionFile fromYAMLFile(const std::filesystem::path& yaml_path);

   static LineageDefinitionFile fromYAMLString(const std::string& yaml_string);

   static LineageDefinitionFile fromYAML(const YAML::Node& yaml_node);
};

}  // namespace silo::preprocessing

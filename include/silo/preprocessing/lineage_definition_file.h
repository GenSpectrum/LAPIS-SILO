#pragma once

#include <filesystem>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace silo::preprocessing {

class LineageName {
  public:
   std::string string;

   bool operator==(const LineageName& other) const;
};

class LineageDefinition {
  public:
   LineageName lineage_name;
   std::vector<LineageName> parents;
};

class LineageDefinitionFile {
  public:
   std::vector<LineageDefinition> lineages;

   static LineageDefinitionFile fromYAMLFile(const std::filesystem::path& yaml_path);

   static LineageDefinitionFile fromYAML(const std::string& yaml_string);
};

}  // namespace silo::preprocessing

#pragma once

#include <filesystem>
#include <vector>

namespace silo::preprocessing {

class LineageName {
  public:
   std::string string;
};

class LineageDefinition {
  public:
   LineageName lineage_name;
   std::vector<LineageName> parent_lineages;
};

class LineageDefinitionFile {
  public:
   std::vector<LineageDefinition> lineages;

   static LineageDefinitionFile fromYAML(const std::string& yaml_string);

   static LineageDefinitionFile fromYAMLFile(const std::filesystem::path& yaml_path);
};

}  // namespace silo::preprocessing

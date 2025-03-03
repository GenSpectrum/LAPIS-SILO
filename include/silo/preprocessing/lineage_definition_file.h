#pragma once

#include <filesystem>
#include <vector>

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

   static LineageDefinitionFile fromYAML(const std::string& yaml_string);
};

}  // namespace silo::preprocessing

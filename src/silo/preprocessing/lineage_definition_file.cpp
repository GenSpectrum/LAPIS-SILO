#include "silo/preprocessing/lineage_definition_file.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "silo/common/panic.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace YAML {
using silo::preprocessing::LineageDefinition;
using silo::preprocessing::LineageDefinitionFile;
using silo::preprocessing::LineageName;

template <>
struct convert<LineageName> {
   static bool decode(const Node& node, LineageName& lineage_name) {
      try {
         lineage_name.string = node.as<std::string>();
      } catch (const YAML::BadConversion& e) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "Could not parse Lineage definition name '{}', as it is not a string.", YAML::Dump(node)
         ));
      }
      return true;
   }
   static Node encode(const LineageName& lineage_name) {
      Node node;
      node = lineage_name.string;
      return node;
   }
};

template <>
struct convert<LineageDefinitionFile> {
   static bool decode(const Node& node, LineageDefinitionFile& lineage_definition) {
      std::vector<LineageDefinition> lineage_definitions;
      for (const auto& entry : node) {
         auto lineage_name = entry.first.as<LineageName>();
         auto parents = entry.second["parents"].as<std::vector<LineageName>>();
         lineage_definitions.emplace_back(lineage_name, parents);
      }
      lineage_definition = LineageDefinitionFile{lineage_definitions};
      return true;
   }
   static Node encode(const LineageDefinitionFile& lineage_definition) {
      Node node;
      for (const auto& lineage : lineage_definition.lineages) {
         node[lineage.lineage_name.string] = lineage.parent_lineages;
      }
      return node;
   }
};
}  // namespace YAML

namespace silo::preprocessing {

LineageDefinitionFile LineageDefinitionFile::fromYAML(const std::string& yaml_string) {
   YAML::Node yaml = YAML::Load(yaml_string);
   LineageDefinitionFile file;
   YAML::convert<LineageDefinitionFile>::decode(yaml, file);
   return file;
}

LineageDefinitionFile LineageDefinitionFile::fromYAMLFile(const std::filesystem::path& yaml_path) {
   std::ifstream file(yaml_path, std::ios::in | std::ios::binary);  // Open file in binary mode
   if (!file) {
      throw std::runtime_error("Could not open the YAML file: " + yaml_path.string());
   }

   std::ostringstream contents;
   contents << file.rdbuf();
   return fromYAML(contents.str());
}

}  // namespace silo::preprocessing
#include "silo/preprocessing/lineage_definition_file.h"

#include <fstream>
#include <unordered_set>

#include <yaml-cpp/yaml.h>

#include "silo/common/panic.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace std {
using silo::preprocessing::LineageName;
template <>
struct hash<LineageName> {
   std::size_t operator()(const LineageName& ln) const {
      return std::hash<std::string>()(ln.string);
   }
};
}  // namespace std

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
      std::unordered_set<LineageName> unique_lineage_definitions;
      for (const auto& entry : node) {
         auto lineage_name = entry.first.as<LineageName>();
         if (unique_lineage_definitions.contains(lineage_name)) {
            throw silo::preprocessing::PreprocessingException(fmt::format(
               "The lineage definitions contain the duplicate lineage '{}'", lineage_name.string
            ));
         }
         if (!entry.second.IsMap()) {
            throw silo::preprocessing::PreprocessingException(fmt::format(
               "The lineage '{}' is not defined as a valid YAML Map in its definition: {}",
               lineage_name.string,
               YAML::Dump(entry.second)
            ));
         }
         if (!entry.second["parents"]) {
            throw silo::preprocessing::PreprocessingException(fmt::format(
               "The lineage '{}' does not contain the field 'parents'", lineage_name.string
            ));
         }
         if (!entry.second["parents"].IsSequence()) {
            throw silo::preprocessing::PreprocessingException(fmt::format(
               "The parents of lineage '{}' are not defined as a YAML Sequence", lineage_name.string
            ));
         }
         if (std::ranges::any_of(entry.second, [](const auto& element) {
                return element.first.template as<std::string>() != "parents";
             })) {
            throw silo::preprocessing::PreprocessingException(fmt::format(
               "The definition of lineage '{}' contains the invalid fields (only 'parents' is "
               "allowed): {}",
               lineage_name.string,
               YAML::Dump(entry.second)
            ));
         }
         unique_lineage_definitions.emplace(lineage_name);
         auto parents = entry.second["parents"].as<std::vector<LineageName>>();
         lineage_definitions.emplace_back(lineage_name, parents);
      }
      lineage_definition = LineageDefinitionFile{lineage_definitions};
      return true;
   }
};
}  // namespace YAML

namespace silo::preprocessing {

bool LineageName::operator==(const LineageName& other) const {
   return string == other.string;
}

LineageDefinitionFile LineageDefinitionFile::fromYAMLFile(const std::filesystem::path& yaml_path) {
   const std::ifstream file(yaml_path, std::ios::in | std::ios::binary);
   if (!file) {
      throw silo::preprocessing::PreprocessingException(
         "Could not open the YAML file: " + yaml_path.string()
      );
   }

   std::ostringstream contents;
   contents << file.rdbuf();
   try {
      return fromYAML(contents.str());
   } catch (YAML::ParserException parser_exception) {
      throw silo::preprocessing::PreprocessingException(
         "The YAML file {} does not contain valid YAML."
      );
   }
}

// If the string does not contain valid YAML, it will throw a YAML::ParserException.
// If the YAML is not of the expected structure, it will throw a different Error about what exactly
// was not of the expected form.
LineageDefinitionFile LineageDefinitionFile::fromYAML(const std::string& yaml_string) {
   const YAML::Node yaml = YAML::Load(yaml_string);
   LineageDefinitionFile file;
   YAML::convert<LineageDefinitionFile>::decode(yaml, file);
   return file;
}

}  // namespace silo::preprocessing

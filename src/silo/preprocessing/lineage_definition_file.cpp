#include "silo/preprocessing/lineage_definition_file.h"

#include <fstream>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

#include "silo/common/fmt_formatters.h"
#include "silo/common/panic.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace YAML {
using silo::common::LineageName;
using silo::preprocessing::LineageDefinition;
using silo::preprocessing::LineageDefinitionFile;

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
   static YAML::Node encode(const LineageName& lineage_name) {
      Node node;
      node = lineage_name.string;
      return node;
   }
};

namespace {

LineageDefinition entryToLineageDefinition(const YAML::detail::iterator_value& entry) {
   auto lineage_name = entry.first.as<LineageName>();
   if (entry.second.IsNull()) {
      return {
         .lineage_name = lineage_name,
         .aliases = std::vector<LineageName>{},
         .parents = std::vector<LineageName>{}
      };
   }
   if (!entry.second.IsMap()) {
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "The lineage '{}' is not defined as a valid YAML Map in its definition: {}",
         lineage_name,
         YAML::Dump(entry.second)
      ));
   }
   if (std::ranges::any_of(entry.second, [](const auto& element) {
          const auto field_name = element.first.template as<std::string>();
          return field_name != "parents" && field_name != "aliases";
       })) {
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "The definition of lineage '{}' may only contain the fields 'parents' and 'aliases', it "
         "also contains invalid fields:\n{}",
         lineage_name,
         YAML::Dump(entry.second)
      ));
   }
   std::vector<LineageName> parents;
   if (entry.second["parents"]) {
      if (!entry.second["parents"].IsSequence()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The parents of lineage '{}' are not defined as a YAML Sequence", lineage_name
         ));
      }
      parents = entry.second["parents"].as<std::vector<LineageName>>();
   }
   std::vector<LineageName> aliases;
   if (entry.second["aliases"]) {
      if (!entry.second["aliases"].IsSequence()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aliases of lineage '{}' are not defined as a YAML Sequence", lineage_name
         ));
      }
      aliases = entry.second["aliases"].as<std::vector<LineageName>>();
   }
   return {.lineage_name = lineage_name, .aliases = aliases, .parents = parents};
}
}  // namespace

template <>
struct convert<LineageDefinitionFile> {
   static bool decode(const Node& node, LineageDefinitionFile& lineage_definition_file) {
      std::vector<LineageDefinition> lineage_definitions;
      for (const auto& entry : node) {
         lineage_definitions.emplace_back(entryToLineageDefinition(entry));
      }
      lineage_definition_file = LineageDefinitionFile{lineage_definitions};
      return true;
   }
};
}  // namespace YAML

namespace silo::preprocessing {

LineageDefinitionFile LineageDefinitionFile::fromYAMLFile(const std::filesystem::path& yaml_path) {
   std::ifstream file(yaml_path, std::ios::in | std::ios::binary);
   if (!file) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Could not open the YAML file: '{}'", yaml_path)
      );
   }

   std::ostringstream contents;
   if (file.peek() != std::ifstream::traits_type::eof()) {
      contents << file.rdbuf();
      if (contents.fail()) {
         throw silo::preprocessing::PreprocessingException(
            fmt::format("Error when reading the YAML file: '{}'", yaml_path)
         );
      }
   }
   try {
      return fromYAML(contents.str());
   } catch (const YAML::ParserException& parser_exception) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The YAML file '{}' does not contain valid YAML.", yaml_path)
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

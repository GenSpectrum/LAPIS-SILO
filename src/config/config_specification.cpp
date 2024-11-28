#include "config/config_specification.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/core/span.hpp>

#include "config/source/command_line_arguments.h"
#include "config/source/environment_variables.h"
#include "config/source/yaml_file.h"
#include "config/config_key_path.h"
#include "silo/common/cons_list.h"
#include "silo/common/fmt_formatters.h"
#include "silo/common/panic.h"
#include "silo/common/string_utils.h"

namespace {
std::string indent(std::string_view indentation, const std::string& str) {
   auto lines = silo::splitBy(str, "\n");
   std::string out{};
   for (const auto& line : lines) {
      // Can't do: out.push_back(std::string { indentation });
      for (const char character : indentation) {
         out.push_back(character);
      }
      for (const char character : line) {
         out.push_back(character);
      }
      out.push_back('\n');
   }
   return out;
}
}  // namespace

namespace silo::config {

std::optional<ConfigValueSpecification> ConfigSpecification::getValueSpecificationFromAmbiguousKey(
   const silo::config::AmbiguousConfigKeyPath& key
) const {
   for (const auto& field : fields) {
      if (key == AmbiguousConfigKeyPath::from(field.key)) {
         return field;
      }
   }
   return std::nullopt;
}

std::optional<ConfigValueSpecification> ConfigSpecification::getValueSpecification(
   const silo::config::ConfigKeyPath& key
) const {
   auto maybe_result = std::find_if(
      fields.begin(),
      fields.end(),
      [&](const ConfigValueSpecification& value_specification) {
         return value_specification.key == key;
      }
   );
   if (maybe_result == fields.end()) {
      return std::nullopt;
   }
   return *maybe_result;
}

std::string ConfigSpecification::helpText() const {
   std::ostringstream help_text;
   help_text << "Usage: " << program_name << " [options...]\n"
             << "   or: silo api|preprocess [options...]\n"
             << "\n"
             << "  Showing the options for " << program_name
             << ". To see the options for the sister\n"
             << "  program, use 'silo api|preprocess --help'.\n"
             << "\n"
             << "  Options override environment variables which override YAML file entries.\n"
             << "  The following options are valid:\n";
   // ^ XX are keys with dot working in YAML? Or have to describe what is meant?
   auto addln = [&help_text](const std::string& line) { help_text << line << "\n"; };

   for (const auto& field_spec : fields) {
      addln("");
      const std::string_view type_text = field_spec.type == ConfigValueType::BOOL
                                            ? " (boolean, the option implies 'true')"
                                            : configValueTypeToString(field_spec.type);
      addln(fmt::format(
         "  {} {}", CommandLineArguments::configKeyPathToString(field_spec.key), type_text
      ));
      addln(fmt::format(
         "     Env var: {}", EnvironmentVariables::configKeyPathToString(field_spec.key)
      ));
      addln(fmt::format("    YAML key: {}", YamlConfig::configKeyPathToString(field_spec.key)));
      addln("\n" + indent(std::string_view{"    "}, std::string{field_spec.help_text}));
      addln(
         field_spec.default_value.has_value()
            ? fmt::format("    Default: {}", field_spec.default_value->toString())
            : "    No default."
      );
   }

   return help_text.str();
}

VerifiedConfigSource ConfigSpecification::getConfigSourceFromDefaults() const {
   VerifiedConfigSource result;
   for (const auto& value_specification : fields) {
      if (value_specification.default_value) {
         result.config_values.emplace(
            value_specification.key, value_specification.default_value.value()
         );
      }
   }
   return result;
}

}  // namespace silo::config

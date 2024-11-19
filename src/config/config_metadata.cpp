#include <config/config_metadata.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/core/span.hpp>

#include <config/config_key_path.h>
#include <config/source/command_line_arguments.h>
#include <config/source/environment_variables.h>
#include <config/source/yaml_file.h>
#include <silo/common/cons_list.h>
#include <silo/common/overloaded.h>
#include <silo/common/panic.h>
#include <silo/common/string_utils.h>

bool ConfigValue::isBool() const {
   return strcmp(type_name, "bool") == 0;
}

std::string indent(std::string_view indentation, const std::string& str) {
   auto lines = silo::splitBy(str, "\n");
   std::string out{};
   for (const auto& line : lines) {
      // Can't do: out.push_back(std::string { indentation });
      for (char chr : indentation) {
         out.push_back(chr);
      }
      for (char chr : line) {
         out.push_back(chr);
      }
      out.push_back('\n');
   }
   return out;
}

void ConfigStruct::collectConfigValues(
   const ConsList<std::string>& parents,
   std::vector<std::pair<ConfigKeyPath, const ConfigValue*>>& fields
) const {
   for (const auto& field : this->fields) {
      ConsList<std::string> parents1 = parents.cons(field.field_name_camel);

      std::visit(
         overloaded{
            [&](const ConfigValue& cval) {
               std::pair<ConfigKeyPath, const ConfigValue*> pair{
                  ConfigKeyPath{parents1.toVecReverse()}, &cval
               };
               fields.push_back(pair);
            },
            [&](const ConfigStruct* cstr) { cstr->collectConfigValues(parents1, fields); }
         },
         field.value
      );
   }
}

std::string ConfigStruct::configContext() const {
   return fmt::format("ConfigStruct {:?}", program_or_struct_name);
}
std::string ConfigStruct::configKeyPathToString(const ConfigKeyPath& /*config_key_path*/) const {
   SILO_UNIMPLEMENTED();
}

/// Returns the default value string template for the given
/// key. Panics (it's OK since this would be an implementation bug
/// in `OverwriteFrom::copy_defaults_from` implementations) if
/// `key` is not found or leads to a sub-struct.
std::optional<std::string> ConfigStruct::getString(const ConfigKeyPath& config_key_path) const {
   SPDLOG_TRACE("getString('{}') on {}", config_key_path.toDebugString(), configContext());

   const ConfigStruct* config_struct = this;

   std::span<const std::string> path{config_key_path.path};

   // iterate down the path
   while (true) {
      SPDLOG_TRACE(
         "  path = '{}', config_struct = {}",
         boost::join(boost::span<const std::string>(path), "/"),
         config_struct->configContext()
      );

      if (path.empty()) {
         SILO_PANIC(
            "config key path {} led to sub-struct {} instead of a leaf field",
            config_key_path.toDebugString(),
            config_struct->program_or_struct_name
         );
      }
      const std::string& key = path[0];  // is this safe? *should* be?
      path = {path.begin() + 1, path.end()};

      // scan the fields for key
      for (auto field : config_struct->fields) {
         if (field.field_name_camel == key) {
            const auto& val = field.value;
            if (std::holds_alternative<ConfigValue>(val)) {
               const auto& cval = std::get<ConfigValue>(val);
               if (path.empty()) {
                  return cval.default_value;
               }
               SILO_PANIC(
                  "key {} in ConfigStruct {} leads to a leaf field "
                  "but path isn't finished: {}",
                  key,
                  config_struct->program_or_struct_name,
                  boost::join(boost::span<const std::string>(path), "/")
               );
            } else if (std::holds_alternative<const ConfigStruct*>(val)) {
               const auto* cstr = std::get<const ConfigStruct*>(val);
               config_struct = cstr;
               goto outer;
            } else {
               SILO_UNREACHABLE();
            }
         }
      }
      SILO_PANIC("key '{}' not found in {}", key, config_struct->configContext());
   outer: { /* continue */
   }
   }
}

const std::vector<std::string>* ConfigStruct::positionalArgs() const {
   SILO_UNIMPLEMENTED();
}

std::vector<std::pair<ConfigKeyPath, const ConfigValue*>> ConfigStruct::configValues() const {
   std::vector<std::pair<ConfigKeyPath, const ConfigValue*>> values;
   collectConfigValues(ConsList<std::string>(), values);
   {
      // Check for duplicates
      std::unordered_set<const ConfigKeyPath*> paths;
      for (const auto& [path, value] : values) {
         if (!paths.insert(&path).second) {
            throw std::runtime_error("duplicate config key path found");
         }
      }
   }
   return values;
}

std::string ConfigStruct::helpText() const {
   std::string program_name = program_or_struct_name;
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

   auto config_values = configValues();

   // For showing the field access paths for the 3 possible sources
   std::array<std::tuple<bool, std::string, std::unique_ptr<ConfigSource>>, 3> sources = {
      std::make_tuple(
         true,
         "",
         std::make_unique<CommandLineArguments>(CommandLineArguments{std::span<const std::string>{}}
         )
      ),
      std::make_tuple(false, "\n    Env var: ", std::make_unique<EnvironmentVariables>()),
      std::make_tuple(false, "  YAML key: ", std::make_unique<YamlFile>())
   };
   for (const auto& [config_key_path, config_value] : config_values) {
      addln("");
      for (const auto& [is_command_line, key_name, source] : sources) {
         // XX this is nigh unreadable. use fmt::format or/and change to if/else from ? :
         addln(
            "  " + key_name + source->configKeyPathToString(config_key_path) +
            (is_command_line ? (config_value->isBool() ? " (boolean, the option implies 'true')"
                                                       : std::string(" ") + config_value->type_name)
                             : "")
         );
      }
      addln("\n" + indent(std::string_view{"    "}, std::string{config_value->help_text}));
      addln(
         config_value->default_value.has_value()
            ? std::string("    Default: ") + config_value->default_value.value()
            : "    No default."
      );
   }

   return help_text.str();
}

#include "config/verified_config_attributes.h"
#include <spdlog/spdlog.h>

#include "config/source/yaml_file.h"
#include "silo/common/panic.h"

namespace silo::config {

std::optional<std::string> VerifiedConfigAttributes::getString(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::STRING) {
         SILO_PANIC(
            "Called getString on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            YamlFile::configKeyPathToString(config_key_path),
            configValueTypeToString(value.getValueType())
         );
      }
      SPDLOG_TRACE(
         "Using for key {} the value {}",
         YamlFile::configKeyPathToString(config_key_path),
         value.toString()
      );
      return get<std::string>(value.value);
   }
   return std::nullopt;
}

std::optional<std::filesystem::path> VerifiedConfigAttributes::getPath(
   const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::PATH) {
         SILO_PANIC(
            "Called getPath on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            YamlFile::configKeyPathToString(config_key_path),
            configValueTypeToString(value.getValueType())
         );
      }
      SPDLOG_TRACE(
         "Using for key {} the value {}",
         YamlFile::configKeyPathToString(config_key_path),
         value.toString()
      );
      return get<std::filesystem::path>(value.value);
   }
   return std::nullopt;
}

std::optional<int32_t> VerifiedConfigAttributes::getInt32(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::INT32) {
         SILO_PANIC(
            "Called getInt32 on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            YamlFile::configKeyPathToString(config_key_path),
            configValueTypeToString(value.getValueType())
         );
      }
      SPDLOG_TRACE(
         "Using for key {} the value {}",
         YamlFile::configKeyPathToString(config_key_path),
         value.toString()
      );
      return get<int32_t>(value.value);
   }
   return std::nullopt;
}

std::optional<uint32_t> VerifiedConfigAttributes::getUint32(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::UINT32) {
         SILO_PANIC(
            "Called getUint32 on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            YamlFile::configKeyPathToString(config_key_path),
            configValueTypeToString(value.getValueType())
         );
      }
      SPDLOG_TRACE(
         "Using for key {} the value {}",
         YamlFile::configKeyPathToString(config_key_path),
         value.toString()
      );
      return std::get<uint32_t>(value.value);
   }
   return std::nullopt;
}

std::optional<uint16_t> VerifiedConfigAttributes::getUint16(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::UINT16) {
         SILO_PANIC(
            "Called getUint16 on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            YamlFile::configKeyPathToString(config_key_path),
            configValueTypeToString(value.getValueType())
         );
      }
      SPDLOG_TRACE(
         "Using for key {} the value {}",
         YamlFile::configKeyPathToString(config_key_path),
         value.toString()
      );
      return std::get<uint16_t>(value.value);
   }
   return std::nullopt;
}

std::optional<bool> VerifiedConfigAttributes::getBool(const ConfigKeyPath& config_key_path) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::BOOL) {
         SILO_PANIC(
            "Called getBool on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            YamlFile::configKeyPathToString(config_key_path),
            configValueTypeToString(value.getValueType())
         );
      }
      SPDLOG_TRACE(
         "Using for key {} the value {}",
         YamlFile::configKeyPathToString(config_key_path),
         value.toString()
      );
      return get<bool>(value.value);
   }
   return std::nullopt;
}

}  // namespace silo::config

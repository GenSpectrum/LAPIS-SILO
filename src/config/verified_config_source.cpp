#include "config/verified_config_source.h"

#include "silo/common/panic.h"

namespace silo::config {

std::optional<std::string> VerifiedConfigSource::getString(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::STRING) {
         SILO_PANIC(
            "Called getString on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            config_key_path.toDebugString(),
            configValueTypeToString(value.getValueType())
         );
      }
      return get<std::string>(value.value);
   }
   return std::nullopt;
}

std::optional<std::filesystem::path> VerifiedConfigSource::getPath(
   const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::PATH) {
         SILO_PANIC(
            "Called getPath on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            config_key_path.toDebugString(),
            configValueTypeToString(value.getValueType())
         );
      }
      return get<std::filesystem::path>(value.value);
   }
   return std::nullopt;
}

std::optional<int32_t> VerifiedConfigSource::getInt32(const ConfigKeyPath& config_key_path) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::INT32) {
         SILO_PANIC(
            "Called getInt32 on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            config_key_path.toDebugString(),
            configValueTypeToString(value.getValueType())
         );
      }
      return get<int32_t>(value.value);
   }
   return std::nullopt;
}

std::optional<uint32_t> VerifiedConfigSource::getUint32(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::UINT32) {
         SILO_PANIC(
            "Called getUint32 on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            config_key_path.toDebugString(),
            configValueTypeToString(value.getValueType())
         );
      }
      return get<uint32_t>(value.value);
   }
   return std::nullopt;
}

std::optional<uint16_t> VerifiedConfigSource::getUint16(const ConfigKeyPath& config_key_path
) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::UINT16) {
         SILO_PANIC(
            "Called getUint16 on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            config_key_path.toDebugString(),
            configValueTypeToString(value.getValueType())
         );
      }
      return get<uint16_t>(value.value);
   }
   return std::nullopt;
}

std::optional<bool> VerifiedConfigSource::getBool(const ConfigKeyPath& config_key_path) const {
   auto value_it = config_values.find(config_key_path);
   if (value_it != config_values.end()) {
      const ConfigValue& value = value_it->second;
      if (value.getValueType() != ConfigValueType::BOOL) {
         SILO_PANIC(
            "Called getBool on a ConfigKeyPath ('{}') that belongs to a value of another "
            "type ({}).",
            config_key_path.toDebugString(),
            configValueTypeToString(value.getValueType())
         );
      }
      return get<bool>(value.value);
   }
   return std::nullopt;
}

}  // namespace silo::config

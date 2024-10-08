#include "silo/common/data_version.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <utility>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "silo/common/panic.h"

namespace silo {

static const std::string TIMESTAMP_FIELD = "timestamp";
static const std::string SERIALIZATION_VERSION_FIELD = "serializationVersion";

DataVersion::Timestamp::Timestamp(std::string value)
    : value(std::move(value)) {}

std::optional<DataVersion::Timestamp> DataVersion::Timestamp::fromString(
   std::string timestamp_string
) {
   if (!std::ranges::all_of(timestamp_string, [](char character) {
          return character >= '0' && character <= '9';
       })) {
      return std::nullopt;
   }
   return DataVersion::Timestamp{timestamp_string};
}

bool DataVersion::Timestamp::operator==(const silo::DataVersion::Timestamp& other) const {
   return this->value == other.value;
}

bool DataVersion::Timestamp::operator<(const DataVersion::Timestamp& other) const {
   return this->value < other.value;
}
bool DataVersion::Timestamp::operator>(const DataVersion::Timestamp& other) const {
   return other < *this;
}
bool DataVersion::Timestamp::operator<=(const DataVersion::Timestamp& other) const {
   return !(other < *this);
}
bool DataVersion::Timestamp::operator>=(const DataVersion::Timestamp& other) const {
   return !(*this < other);
}

DataVersion DataVersion::mineDataVersion() {
   const auto now = std::chrono::system_clock::now();
   const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
   const auto current_timestamp = Timestamp::fromString(std::to_string(now_as_time_t));
   ASSERT(current_timestamp.has_value());
   return DataVersion{current_timestamp.value(), {CURRENT_SILO_SERIALIZATION_VERSION}};
}

DataVersion::DataVersion(
   DataVersion::Timestamp timestamp,
   SerializationVersion serialization_version
)
    : timestamp(std::move(timestamp)),
      serialization_version(serialization_version) {}

std::string DataVersion::toString() const {
   return fmt::format(
      "{{{}: {}, {}: {}}}",
      TIMESTAMP_FIELD,
      timestamp.value,
      SERIALIZATION_VERSION_FIELD,
      serialization_version.value
   );
}

bool DataVersion::operator==(const DataVersion& other) const {
   return this->timestamp == other.timestamp &&
          this->serialization_version.value == other.serialization_version.value;
}
bool DataVersion::operator!=(const DataVersion& other) const {
   return !(*this == other);
}

bool DataVersion::operator<(const DataVersion& other) const {
   return this->timestamp < other.timestamp ||
          (this->timestamp == other.timestamp &&
           this->serialization_version.value < other.serialization_version.value);
}
bool DataVersion::operator>(const DataVersion& other) const {
   return other < *this;
}
bool DataVersion::operator<=(const DataVersion& other) const {
   return !(other < *this);
}
bool DataVersion::operator>=(const DataVersion& other) const {
   return !(*this < other);
}

bool DataVersion::isCompatibleVersion() const {
   return this->serialization_version.value == CURRENT_SILO_SERIALIZATION_VERSION.value;
}

DataVersion::Timestamp DataVersion::getTimestamp() const {
   return timestamp;
}

std::optional<DataVersion> DataVersion::fromFile(const std::filesystem::path& filename) {
   YAML::Node node = YAML::LoadFile(filename);
   if (!node.IsDefined() || node.IsNull()) {
      return std::nullopt;
   }
   const std::string timestamp_string =
      node.IsScalar() ? node.as<std::string>() : node[TIMESTAMP_FIELD].as<std::string>();
   auto timestamp = Timestamp::fromString(timestamp_string);
   if (!timestamp) {
      return std::nullopt;
   }
   try {
      const uint32_t serialization_version =
         node.IsMap() && node[SERIALIZATION_VERSION_FIELD].IsDefined()
            ? node[SERIALIZATION_VERSION_FIELD].as<uint32_t>()
            : 0;
      return DataVersion{*timestamp, {serialization_version}};
   } catch (YAML::TypedBadConversion<uint32_t>&) {
      SPDLOG_WARN(
         "The serialization version {} in {} is not a valid 32-bit unsigned integer.",
         node[SERIALIZATION_VERSION_FIELD].as<std::string>(),
         filename.string()
      );
      return std::nullopt;
   }
}

void DataVersion::saveToFile(std::ofstream& save_file) const {
   YAML::Node yaml_representation;
   yaml_representation[TIMESTAMP_FIELD] = timestamp.value;
   yaml_representation[SERIALIZATION_VERSION_FIELD] = serialization_version.value;
   save_file << Dump(yaml_representation);
}

}  // namespace silo
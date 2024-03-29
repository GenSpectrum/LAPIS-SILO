#include "silo/common/data_version.h"

#include <algorithm>
#include <chrono>
#include <utility>

namespace silo {

DataVersion DataVersion::mineDataVersion() {
   const auto now = std::chrono::system_clock::now();
   const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
   return DataVersion{std::to_string(now_as_time_t)};
}

DataVersion::DataVersion(std::string data_version)
    : data_version(std::move(data_version)) {}

std::string DataVersion::toString() const {
   return data_version;
}

bool DataVersion::operator==(const DataVersion& other) const {
   return this->data_version == other.data_version;
}
bool DataVersion::operator!=(const DataVersion& other) const {
   return !(*this == other);
}

// Lexicographical ordering with the empty string as the lowest order
bool DataVersion::operator<(const DataVersion& other) const {
   return this->data_version < other.data_version;
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

std::optional<DataVersion> DataVersion::fromString(const std::string& string) {
   if (std::all_of(string.begin(), string.end(), [](char character) {
          return character >= '0' && character <= '9';
       })) {
      return DataVersion{string};
   }
   return std::nullopt;
}

}  // namespace silo
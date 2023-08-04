#include "silo/common/data_version.h"

#include <chrono>
#include <utility>

namespace silo {

DataVersion DataVersion::mineDataVersion() {
   const auto now = std::chrono::system_clock::now();
   const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
   return DataVersion(std::to_string(now_as_time_t));
}

DataVersion::DataVersion(std::string data_version)
    : data_version(std::move(data_version)) {}

std::string DataVersion::toString() const {
   return data_version;
}

}  // namespace silo
#pragma once

#include <optional>
#include <string>

namespace silo {

class DataVersion {
   friend class Database;

  private:
   std::string data_version;

  public:
   [[nodiscard]] std::string toString() const;

   bool operator==(const DataVersion& other) const;
   bool operator!=(const DataVersion& other) const;
   bool operator<(const DataVersion& other) const;
   bool operator>(const DataVersion& other) const;
   bool operator<=(const DataVersion& other) const;
   bool operator>=(const DataVersion& other) const;

   static DataVersion mineDataVersion();

   static std::optional<DataVersion> fromString(const std::string& string);

  private:
   explicit DataVersion(std::string data_version);
};

}  // namespace silo

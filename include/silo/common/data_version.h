#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace silo {

class DataVersion {
   friend class Database;

  public:
   class SerializationVersion {
     public:
      uint32_t value;
   };

   static constexpr SerializationVersion CURRENT_SILO_SERIALIZATION_VERSION{5};

   class Timestamp {
     public:
      std::string value;
      static std::optional<Timestamp> fromString(std::string timestamp_string);

      [[nodiscard]] bool operator==(const Timestamp& other) const;
      [[nodiscard]] bool operator<(const Timestamp& other) const;
      [[nodiscard]] bool operator>(const Timestamp& other) const;
      [[nodiscard]] bool operator<=(const Timestamp& other) const;
      [[nodiscard]] bool operator>=(const Timestamp& other) const;

     private:
      explicit Timestamp(std::string value);
   };

  private:
   Timestamp timestamp;
   SerializationVersion serialization_version;

  public:
   [[nodiscard]] std::string toString() const;

   [[nodiscard]] bool operator==(const DataVersion& other) const;
   [[nodiscard]] bool operator!=(const DataVersion& other) const;
   [[nodiscard]] bool operator<(const DataVersion& other) const;
   [[nodiscard]] bool operator>(const DataVersion& other) const;
   [[nodiscard]] bool operator<=(const DataVersion& other) const;
   [[nodiscard]] bool operator>=(const DataVersion& other) const;

   [[nodiscard]] bool isCompatibleVersion() const;

   Timestamp getTimestamp() const;

   static DataVersion mineDataVersion();

   static std::optional<DataVersion> fromFile(const std::filesystem::path& filename);

   void saveToFile(std::ofstream& save_file) const;

  private:
   explicit DataVersion(Timestamp timestamp, SerializationVersion serialization_version);
};

}  // namespace silo

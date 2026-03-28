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

   static const SerializationVersion CURRENT_SILO_SERIALIZATION_VERSION;

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

   [[nodiscard]] Timestamp getTimestamp() const;

   static DataVersion mineDataVersion();

   static std::optional<DataVersion> fromFile(const std::filesystem::path& file_path);

   void saveToFile(const std::filesystem::path& save_file) const;

  private:
   explicit DataVersion(Timestamp timestamp, SerializationVersion serialization_version);
};

}  // namespace silo

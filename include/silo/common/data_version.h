#ifndef SILO_INCLUDE_SILO_COMMON_DATAVERSION_H_
#define SILO_INCLUDE_SILO_COMMON_DATAVERSION_H_

#include <string>

class DataVersion {
  public:
   explicit DataVersion(const std::string& data_version = "");
   [[nodiscard]] std::string toString() const;

   static std::string mineDataVersion();

  private:
   std::string data_version;
};

#endif  // SILO_INCLUDE_SILO_COMMON_DATAVERSION_H_

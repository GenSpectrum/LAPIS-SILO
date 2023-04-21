#ifndef SILO_INCLUDE_SILO_CONFIG_DATABASE_CONFIG_READER_H_
#define SILO_INCLUDE_SILO_CONFIG_DATABASE_CONFIG_READER_H_

#include <filesystem>

#include "silo/config/database_config.h"

namespace silo {

class DatabaseConfigReader {
  public:
   static DatabaseConfig readConfig(const std::filesystem::path& config_path);
};

};  // namespace silo

#endif  // SILO_INCLUDE_SILO_CONFIG_DATABASE_CONFIG_READER_H_

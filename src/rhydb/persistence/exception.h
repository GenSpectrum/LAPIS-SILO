#pragma once

#include <stdexcept>
#include <string>

namespace rhydb::persistence {

class LoadDatabaseException : public std::runtime_error {
  public:
   explicit LoadDatabaseException(const std::string& error_message);
};

class SaveDatabaseException : public std::runtime_error {
  public:
   explicit SaveDatabaseException(const std::string& error_message);
};

}  // namespace rhydb::persistence

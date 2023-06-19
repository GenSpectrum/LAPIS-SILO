#ifndef SILO_LOADDATABASEEXCEPTION_H
#define SILO_LOADDATABASEEXCEPTION_H

#include <iostream>
#include <string>

namespace silo::persistence {

class LoadDatabaseException : public std::runtime_error {
  public:
   explicit LoadDatabaseException(const std::string& error_message);
};

class SaveDatabaseException : public std::runtime_error {
  public:
   explicit SaveDatabaseException(const std::string& error_message);
};

}  // namespace silo::persistence

#endif  // SILO_LOADDATABASEEXCEPTION_H

#ifndef SILO_LOADDATABASEEXCEPTION_H
#define SILO_LOADDATABASEEXCEPTION_H

#include <iostream>
#include <string>

namespace silo {
namespace persistence {

class LoadDatabaseException : public std::runtime_error {
  public:
   explicit LoadDatabaseException(const std::string& error_message);
};

}  // namespace persistence
}  // namespace silo

#endif  // SILO_LOADDATABASEEXCEPTION_H

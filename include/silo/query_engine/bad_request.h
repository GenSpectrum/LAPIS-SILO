#pragma once

#include <stdexcept>
#include <string>

#define CHECK_SILO_QUERY(condition, message) \
   if (!(condition)) {                       \
      throw silo::BadRequest(message);       \
   }

namespace silo {

class [[maybe_unused]] BadRequest : public std::runtime_error {
  public:
   explicit BadRequest(const std::string& error_message);
};

}  // namespace silo

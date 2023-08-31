#pragma once

#include <stdexcept>
#include <string>

namespace silo {

class PreprocessingException : public std::runtime_error {
  public:
   explicit PreprocessingException(const std::string& error_message);
};

}  // namespace silo

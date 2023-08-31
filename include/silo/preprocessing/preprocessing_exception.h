#pragma once

#include <iostream>
#include <stdexcept>

namespace silo {

class PreprocessingException : public std::runtime_error {
  public:
   explicit PreprocessingException(const std::string& error_message);
};

}  // namespace silo

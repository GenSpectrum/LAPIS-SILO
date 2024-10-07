#pragma once

#include <stdexcept>
#include <string>

namespace silo::sequence_file_reader {

class SamFormatException : public std::runtime_error {
  public:
   explicit SamFormatException(const std::string& error_message)
       : std::runtime_error(error_message.c_str()) {};
};

}  // namespace silo::sequence_file_reader

#ifndef SILO_INCLUDE_SILO_PREPROCESSING_PREPROCESSING_EXCEPTION_H_
#define SILO_INCLUDE_SILO_PREPROCESSING_PREPROCESSING_EXCEPTION_H_

#include <iostream>
#include <stdexcept>

namespace silo {

class PreprocessingException : public std::runtime_error {
  public:
   explicit PreprocessingException(const std::string& error_message);
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_PREPROCESSING_PREPROCESSING_EXCEPTION_H_

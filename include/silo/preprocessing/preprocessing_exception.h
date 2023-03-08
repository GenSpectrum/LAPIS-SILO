#ifndef SILO_INCLUDE_SILO_PREPROCESSING_PREPROCESSING_EXCEPTION_H_
#define SILO_INCLUDE_SILO_PREPROCESSING_PREPROCESSING_EXCEPTION_H_

#include <iostream>

class [[maybe_unused]] PreprocessingException : public std::runtime_error {
   public:
   [[maybe_unused]] PreprocessingException(const std::string& error_message);
};

#endif //SILO_INCLUDE_SILO_PREPROCESSING_PREPROCESSING_EXCEPTION_H_

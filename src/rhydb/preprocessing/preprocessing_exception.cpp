#include "rhydb/preprocessing/preprocessing_exception.h"

#include <string>

namespace rhydb::preprocessing {

PreprocessingException::PreprocessingException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace rhydb::preprocessing

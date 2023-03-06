
#include <silo/preprocessing/preprocessing_exception.h>
[[maybe_unused]] PreprocessingException::PreprocessingException(const std::string& error_message) : std::runtime_error(error_message.c_str()) {
}
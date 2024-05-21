#include "silo/file_reader/fasta_format_exception.h"

#include <stdexcept>
#include <string>

namespace silo {

FastaFormatException::FastaFormatException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo
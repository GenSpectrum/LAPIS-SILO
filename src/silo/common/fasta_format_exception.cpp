#include "silo/common/fasta_format_exception.h"

namespace silo {

FastaFormatException::FastaFormatException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo
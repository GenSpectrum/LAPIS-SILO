//
// Created by Alexander Taepper on 02.05.23.
//

#ifndef SILO_FASTA_FORMAT_EXCEPTION_H
#define SILO_FASTA_FORMAT_EXCEPTION_H

#include <iostream>

namespace silo {

class FastaFormatException : public std::runtime_error {
  public:
   explicit FastaFormatException(const std::string& error_message);
};

}  // namespace silo

#endif  // SILO_FASTA_FORMAT_EXCEPTION_H

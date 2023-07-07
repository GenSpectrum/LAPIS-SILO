#ifndef SILO_FASTA_READER_H
#define SILO_FASTA_READER_H

#include <filesystem>
#include <iostream>
#include <string>

#include "silo/common/input_stream_wrapper.h"

namespace silo {
class FastaReader {
  private:
   silo::InputStreamWrapper in_file;

   bool populateKey(std::string& key);

  public:
   explicit FastaReader(const std::filesystem::path& in_file_name);

   bool nextKey(std::string& key);

   bool next(std::string& key, std::string& genome);

   void reset();
};
}  // namespace silo

#endif  // SILO_FASTA_READER_H

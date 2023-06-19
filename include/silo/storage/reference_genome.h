#ifndef SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
#define SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_

#include <filesystem>
#include <string>
#include <vector>

namespace silo {

struct ReferenceGenome {
   std::vector<std::string> genome;

   explicit ReferenceGenome(std::vector<std::string> reference_genome);

   static ReferenceGenome readFromFile(const std::filesystem::path& reference_genome_file);
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_

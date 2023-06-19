#include "silo/storage/reference_genome.h"

#include <spdlog/spdlog.h>
#include <fstream>

#include "silo/persistence/exception.h"

namespace silo {

ReferenceGenome::ReferenceGenome(std::vector<std::string> reference_genome)
    : genome(std::move(reference_genome)) {}

ReferenceGenome ReferenceGenome::readFromFile(const std::filesystem::path& reference_genome_path) {
   if (!std::filesystem::exists(reference_genome_path)) {
      throw std::filesystem::filesystem_error(
         "Global reference genome file " + reference_genome_path.relative_path().string() +
            " does not exist",
         std::error_code()
      );
   }

   SPDLOG_INFO(
      "Read reference genome from file: {}", reference_genome_path.relative_path().string()
   );
   std::ifstream reference_file(reference_genome_path.string());
   std::vector<std::string> global_reference;
   while (true) {
      std::string line;
      if (!getline(reference_file, line, '\n')) {
         break;
      }
      if (line.find('N') != std::string::npos) {
         throw persistence::LoadDatabaseException("No N in reference genome allowed.");
      }
      global_reference.push_back(line);
   }
   if (global_reference.empty()) {
      throw persistence::LoadDatabaseException("No genome in " + reference_genome_path.string());
   }
   return ReferenceGenome(global_reference);
}

}  // namespace silo
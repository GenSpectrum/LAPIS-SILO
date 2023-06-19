#include "silo/storage/reference_genome.h"

#include <spdlog/spdlog.h>
#include <fstream>

#include "silo/persistence/exception.h"

namespace silo {

ReferenceGenome::ReferenceGenome(std::vector<std::string> genome_segments)
    : genome_segments(std::move(genome_segments)) {}

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
   std::vector<std::string> reference_genome_segments;
   while (true) {
      std::string line;
      if (!getline(reference_file, line, '\n')) {
         break;
      }
      if (line.find('N') != std::string::npos) {
         throw persistence::LoadDatabaseException("No N in reference genome allowed.");
      }
      reference_genome_segments.push_back(line);
   }
   if (reference_genome_segments.empty()) {
      throw persistence::LoadDatabaseException("No genome in " + reference_genome_path.string());
   }
   return ReferenceGenome(reference_genome_segments);
}

}  // namespace silo
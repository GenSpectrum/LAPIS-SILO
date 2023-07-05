#include "silo/storage/reference_genomes.h"

#include <fstream>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/persistence/exception.h"

namespace silo {

ReferenceGenomes::ReferenceGenomes(
   std::unordered_map<std::string, std::string> nucleotide_sequences,
   std::unordered_map<std::string, std::string> aa_sequences
)
    : nucleotide_sequences(std::move(nucleotide_sequences)),
      aa_sequences(std::move(aa_sequences)) {}

namespace {

ReferenceGenomes readFromJson(const std::filesystem::path& reference_genomes_path) {
   std::unordered_map<std::string, std::string> nucleotide_sequences;
   std::unordered_map<std::string, std::string> aa_sequences;
   nlohmann::json reference_genomes_json;
   std::ifstream(reference_genomes_path) >> reference_genomes_json;

   const nlohmann::json nuc_seq_json = reference_genomes_json["nucleotide_sequences"];
   const nlohmann::json aa_seq_json = reference_genomes_json["genes"];

   for (const auto& [key, value] : nuc_seq_json.items()) {
      if (value.is_array()) {
         SPDLOG_INFO("Got array, expected object: " + value.dump());
         continue;
      }
      if (!value.contains("name") || !value["name"].is_string()) {
         SPDLOG_INFO(
            "Expected object to contain the key 'name' with string value, got: " + value.dump()
         );
         continue;
      }
      if (!value.contains("sequence") || !value["sequence"].is_string()) {
         "Expected object to contain the key 'sequence' with string value, got: " + value.dump();
         continue;
      }
      nucleotide_sequences[value["name"]] = value["sequence"];
   }

   for (const auto& [key, value] : aa_seq_json.items()) {
      if (value.is_array()) {
         SPDLOG_INFO("Got array, expected object: " + value.dump());
         continue;
      }
      if (!value.contains("name") || !value["name"].is_string()) {
         "Expected object to contain the key 'name' with string value, got: " + value.dump();
         continue;
      }
      if (!value.contains("sequence") || !value["sequence"].is_string()) {
         "Expected object to contain the key 'sequence' with string value, got: " + value.dump();
         continue;
      }
      aa_sequences[value["name"]] = value["sequence"];
   }
   return ReferenceGenomes{nucleotide_sequences, aa_sequences};
}

}  // namespace

ReferenceGenomes ReferenceGenomes::readFromFile(const std::filesystem::path& reference_genomes_path
) {
   if (!std::filesystem::exists(reference_genomes_path)) {
      throw std::filesystem::filesystem_error(
         "Reference genomes file " + reference_genomes_path.relative_path().string() +
            " does not exist",
         std::error_code()
      );
   }

   if (reference_genomes_path.extension() != ".json") {
      throw std::filesystem::filesystem_error(
         "Reference genomes file " + reference_genomes_path.relative_path().string() +
            " is not a json file",
         std::error_code()
      );
   }

   SPDLOG_INFO(
      "Read pango lineage alias from file: {}", reference_genomes_path.relative_path().string()
   );

   return readFromJson(reference_genomes_path);
}

}  // namespace silo
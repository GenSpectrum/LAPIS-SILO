#include "silo/storage/reference_genomes.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <system_error>
#include <utility>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/preprocessing/preprocessing_exception.h"

template <>
struct nlohmann::adl_serializer<silo::ReferenceGenomes> {
   // NOLINTNEXTLINE(readability-identifier-naming)
   static void to_json(nlohmann::json& js_object, const silo::ReferenceGenomes& reference_genomes) {
      nlohmann::json nucleotide_sequences_json;
      for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
         nucleotide_sequences_json.push_back({{"name", name}, {"sequence", sequence}});
      }
      js_object["nucleotide_sequences"] = std::move(nucleotide_sequences_json);

      nlohmann::json aa_sequences_json;
      for (const auto& [name, sequence] : reference_genomes.raw_aa_sequences) {
         aa_sequences_json.push_back({{"name", name}, {"sequence", sequence}});
      }
      js_object["genes"] = std::move(aa_sequences_json);
   }
};

namespace silo {

ReferenceGenomes::ReferenceGenomes(
   std::unordered_map<std::string, std::string> raw_nucleotide_sequences_,
   std::unordered_map<std::string, std::string> raw_aa_sequences_
)
    : raw_nucleotide_sequences(std::move(raw_nucleotide_sequences_)),
      raw_aa_sequences(std::move(raw_aa_sequences_)) {
   for (const auto& [sequence_name, raw_nucleotide_sequence] : raw_nucleotide_sequences) {
      std::vector<NUCLEOTIDE_SYMBOL> nucleotide_sequence;
      for (const char character : raw_nucleotide_sequence) {
         auto symbol = charToNucleotideSymbol(character);

         if (!symbol.has_value()) {
            throw PreprocessingException(
               "Nucleotide sequence with name " + sequence_name +
               " contains illegal amino acid code: " + std::to_string(character)
            );
         }

         nucleotide_sequence.push_back(*symbol);
      }
      nucleotide_sequences[sequence_name] = nucleotide_sequence;
   }

   for (const auto& [sequence_name, raw_aa_sequence] : raw_aa_sequences) {
      std::vector<AA_SYMBOL> aa_sequence;

      for (const char character : raw_aa_sequence) {
         auto symbol = charToAASymbol(character);

         if (!symbol.has_value()) {
            throw PreprocessingException(
               "Amino Acid sequence with name " + sequence_name +
               " contains illegal amino acid code: " + std::to_string(character)
            );
         }

         aa_sequence.push_back(*symbol);
      }
      aa_sequences[sequence_name] = aa_sequence;
   }
}

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
         SPDLOG_INFO(
            "Expected object to contain the key 'sequence' with string value, got: " + value.dump()
         );
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
         SPDLOG_INFO(
            "Expected object to contain the key 'name' with string value, got: " + value.dump()
         );
         continue;
      }
      if (!value.contains("sequence") || !value["sequence"].is_string()) {
         SPDLOG_INFO(
            "Expected object to contain the key 'sequence' with string value, got: " + value.dump()
         );
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
      "Read reference genomes from file: {}", reference_genomes_path.relative_path().string()
   );

   return readFromJson(reference_genomes_path);
}

void ReferenceGenomes::writeToFile(const std::filesystem::path& reference_genomes_path) const {
   if (reference_genomes_path.extension() != ".json") {
      throw std::filesystem::filesystem_error(
         "Reference genomes file " + reference_genomes_path.relative_path().string() +
            " is not a json file",
         std::error_code()
      );
   }

   SPDLOG_INFO(
      "Write reference genomes to file: {}", reference_genomes_path.relative_path().string()
   );
   const nlohmann::json reference_genomes_json = *this;
   std::ofstream(reference_genomes_path) << reference_genomes_json.dump(4);
}

}  // namespace silo
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
      js_object["nucleotideSequences"] = std::move(nucleotide_sequences_json);

      nlohmann::json aa_sequences_json;
      for (const auto& [name, sequence] : reference_genomes.raw_aa_sequences) {
         aa_sequences_json.push_back({{"name", name}, {"sequence", sequence}});
      }
      js_object["genes"] = std::move(aa_sequences_json);
   }
};

namespace silo {

ReferenceGenomes::ReferenceGenomes(
   std::map<std::string, std::string>&& raw_nucleotide_sequences_,
   std::map<std::string, std::string>&& raw_aa_sequences_
)
    : raw_nucleotide_sequences(std::move(raw_nucleotide_sequences_)),
      raw_aa_sequences(std::move(raw_aa_sequences_)) {
   for (const auto& [sequence_name, raw_nucleotide_sequence] : raw_nucleotide_sequences) {
      std::vector<Nucleotide::Symbol> nucleotide_sequence;
      for (const char character : raw_nucleotide_sequence) {
         auto symbol = Nucleotide::charToSymbol(character);

         if (!symbol.has_value()) {
            throw preprocessing::PreprocessingException(
               "Nucleotide sequence with name " + sequence_name +
               " contains illegal amino acid code: " + std::to_string(character)
            );
         }

         nucleotide_sequence.push_back(*symbol);
      }
      nucleotide_sequences[sequence_name] = nucleotide_sequence;
   }

   for (const auto& [sequence_name, raw_aa_sequence] : raw_aa_sequences) {
      aa_sequences[sequence_name] = stringToVector<AminoAcid>(raw_aa_sequence);
   }
}

namespace {

ReferenceGenomes readFromJson(const std::filesystem::path& reference_genomes_path) {
   std::map<std::string, std::string> nucleotide_sequences;
   std::map<std::string, std::string> aa_sequences;
   nlohmann::json reference_genomes_json;
   std::ifstream(reference_genomes_path) >> reference_genomes_json;

   if (!reference_genomes_json.contains("nucleotideSequences")) {
      throw preprocessing::PreprocessingException(
         "Reference genomes file does not contain key 'nucleotideSequences'"
      );
   }
   if (!reference_genomes_json.contains("genes")) {
      throw preprocessing::PreprocessingException(
         "Reference genomes file does not contain key 'genes'"
      );
   }

   const nlohmann::json nuc_seq_json = reference_genomes_json["nucleotideSequences"];
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
   return ReferenceGenomes{std::move(nucleotide_sequences), std::move(aa_sequences)};
}

}  // namespace

ReferenceGenomes ReferenceGenomes::readFromFile(const std::filesystem::path& reference_genomes_path
) {
   if (!std::filesystem::exists(reference_genomes_path)) {
      throw std::filesystem::filesystem_error(
         "Reference genomes file " + reference_genomes_path.string() + " does not exist",
         std::error_code()
      );
   }

   if (reference_genomes_path.extension() != ".json") {
      throw std::filesystem::filesystem_error(
         "Reference genomes file " + reference_genomes_path.string() + " is not a json file",
         std::error_code()
      );
   }

   SPDLOG_INFO("Read reference genomes from file: {}", reference_genomes_path.string());

   return readFromJson(reference_genomes_path);
}

void ReferenceGenomes::writeToFile(const std::filesystem::path& reference_genomes_path) const {
   if (reference_genomes_path.extension() != ".json") {
      throw std::filesystem::filesystem_error(
         "Reference genomes file " + reference_genomes_path.string() + " is not a json file",
         std::error_code()
      );
   }

   SPDLOG_INFO("Write reference genomes to file: {}", reference_genomes_path.string());
   const nlohmann::json reference_genomes_json = *this;
   std::ofstream(reference_genomes_path) << reference_genomes_json.dump(4);
}

template <>
std::vector<std::string> ReferenceGenomes::getSequenceNames<Nucleotide>() const {
   std::vector<std::string> result;
   std::transform(
      raw_nucleotide_sequences.begin(),
      raw_nucleotide_sequences.end(),
      std::back_inserter(result),
      [](auto& pair) { return pair.first; }
   );
   return result;
}

template <>
std::vector<std::string> ReferenceGenomes::getSequenceNames<AminoAcid>() const {
   std::vector<std::string> result;
   std::transform(
      raw_aa_sequences.begin(),
      raw_aa_sequences.end(),
      std::back_inserter(result),
      [](auto& pair) { return pair.first; }
   );
   return result;
}

template <>
std::map<std::string, std::string> ReferenceGenomes::getRawSequenceMap<Nucleotide>() const {
   return raw_nucleotide_sequences;
}

template <>
std::map<std::string, std::string> ReferenceGenomes::getRawSequenceMap<AminoAcid>() const {
   return raw_aa_sequences;
}

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> ReferenceGenomes::stringToVector(const std::string& string
) {
   std::vector<typename SymbolType::Symbol> sequence_vector;

   for (const char character : string) {
      auto symbol = SymbolType::charToSymbol(character);

      if (!symbol.has_value()) {
         throw preprocessing::PreprocessingException(fmt::format(
            "{} sequence with illegal {} code: {}",
            SymbolType::SYMBOL_NAME,
            SymbolType::SYMBOL_NAME_LOWER_CASE,
            std::to_string(character)
         ));
      }

      sequence_vector.push_back(*symbol);
   }
   return sequence_vector;
}

template <>
std::vector<silo::Nucleotide::Symbol> ReferenceGenomes::stringToVector<silo::Nucleotide>(
   const std::string& string
);

template <>
std::string ReferenceGenomes::vectorToString<Nucleotide>(
   const std::vector<Nucleotide::Symbol>& vector
) {
   std::string sequence_string;
   sequence_string.reserve(vector.size());

   for (const typename Nucleotide::Symbol symbol : vector) {
      auto character = Nucleotide::symbolToChar(symbol);
      sequence_string += character;
   }
   return sequence_string;
}

template <>
std::string ReferenceGenomes::vectorToString<AminoAcid>(const std::vector<AminoAcid::Symbol>& vector
);

}  // namespace silo
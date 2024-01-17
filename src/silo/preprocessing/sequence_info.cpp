#include "silo/preprocessing/sequence_info.h"

#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/reference_genomes.h"

namespace silo::preprocessing {

SequenceInfo::SequenceInfo(const silo::ReferenceGenomes& reference_genomes) {
   for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
      nuc_sequence_names.push_back(name);
   }
   for (const auto& [name, sequence] : reference_genomes.raw_aa_sequences) {
      aa_sequence_names.push_back(name);
   }
}

std::vector<std::string> SequenceInfo::getSequenceSelects() {
   std::vector<std::string> sequence_selects;
   sequence_selects.reserve(nuc_sequence_names.size() + aa_sequence_names.size());
   for (const std::string& name : nuc_sequence_names) {
      sequence_selects.emplace_back(fmt::format(
         "{0}(alignedNucleotideSequences.{1}, "
         "'{1}') as nuc_{1}",
         preprocessing::PreprocessingDatabase::COMPRESS_NUC,
         name
      ));
   }
   for (const std::string& name : aa_sequence_names) {
      sequence_selects.emplace_back(fmt::format(
         "{0}(alignedAminoAcidSequences.{1}, "
         "'{1}') as gene_{1}",
         preprocessing::PreprocessingDatabase::COMPRESS_AA,
         name
      ));
   }
   return sequence_selects;
}

void SequenceInfo::validate(
   duckdb::Connection& connection,
   const std::filesystem::path& input_filename
) const {
   auto result = connection.Query(fmt::format(
      "SELECT json_keys(alignedNucleotideSequences), json_keys(alignedAminoAcidSequences) "
      "FROM "
      "'{}' LIMIT 1; ",
      input_filename.string()
   ));
   if (result->HasError()) {
      throw silo::preprocessing::PreprocessingException(
         "Preprocessing exception when retrieving the fields 'alignedNucleotideSequences' and "
         "'alignedAminoAcidSequences', duckdb threw with error: " +
         result->GetError()
      );
   }
   if (result->RowCount() == 0) {
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "File {} is empty, which must not be empty at this point", input_filename.string()
      ));
   }
   if (result->RowCount() > 1) {
      throw silo::preprocessing::PreprocessingException(
         "Internal exception, expected Row Count=1, actual " + std::to_string(result->RowCount())
      );
   }

   auto nuc_sequence_names_to_validate = extractStringListValue(*result, 0, 0);
   auto aa_sequence_names_to_validate = extractStringListValue(*result, 0, 1);

   for (const std::string& name : nuc_sequence_names_to_validate) {
      if (std::find(nuc_sequence_names.begin(), nuc_sequence_names.end(), name) == nuc_sequence_names.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aligned nucleotide sequence {} which is contained in the input file {} is "
            "not contained in the reference sequences.",
            name,
            input_filename.string()
         ));
      }
   }
   for (const std::string& name : nuc_sequence_names) {
      if (std::find(nuc_sequence_names_to_validate.begin(), nuc_sequence_names_to_validate.end(), name)
          == nuc_sequence_names_to_validate.end()) {
         // TODO(#220) handle the cases when segments are left out appropriately
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aligned nucleotide sequence {} which is contained in the reference sequences is "
            "not contained in the input file {}.",
            name,
            input_filename.string()
         ));
      }
   }
   for (const std::string& name : aa_sequence_names_to_validate) {
      if (std::find(aa_sequence_names.begin(), aa_sequence_names.end(), name) == aa_sequence_names.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aligned amino acid sequence {} which is contained in the input file {} is "
            "not contained in the reference sequences.",
            name,
            input_filename.string()
         ));
      }
   }
   for (const std::string& name : aa_sequence_names) {
      if (std::find(aa_sequence_names_to_validate.begin(), aa_sequence_names_to_validate.end(), name)
          == aa_sequence_names_to_validate.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aligned amino acid sequence {} which is contained in the reference sequences is "
            "not contained in the input file {}.",
            name,
            input_filename.string()
         ));
      }
   }
}

}  // namespace silo::preprocessing
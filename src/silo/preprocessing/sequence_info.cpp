#include "silo/preprocessing/sequence_info.h"

#include <duckdb.hpp>

#include "silo/preprocessing/metadata_info.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"

namespace silo::preprocessing {

namespace {
void validateStruct(
   const std::vector<std::string>& names_to_validate,
   const std::vector<std::string>& names_to_validate_against,
   std::string name_type,
   const std::filesystem::path& input_filename
) {
   for (const std::string& name : names_to_validate) {
      if (std::ranges::find(names_to_validate_against, name) == names_to_validate_against.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The {} {} which is contained in the input file {} is "
            "not contained in the reference sequences.",
            name_type,
            name,
            input_filename.string()
         ));
      }
   }
   for (const std::string& name : names_to_validate_against) {
      if (std::ranges::find(names_to_validate, name) == names_to_validate.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The {} {} which is contained in the reference "
            "sequences is not contained in the input file {}.",
            name_type,
            name,
            input_filename.string()
         ));
      }
   }
}
}  // namespace

void SequenceInfo::validateNdjsonFile(
   const silo::ReferenceGenomes& reference_genomes,
   const std::filesystem::path& input_filename
) {
   duckdb::DuckDB duck_db(nullptr);
   duckdb::Connection connection(duck_db);
   const std::vector<std::string>& nuc_sequence_names =
      reference_genomes.getSequenceNames<Nucleotide>();
   const std::vector<std::string>& aa_sequence_names =
      reference_genomes.getSequenceNames<AminoAcid>();
   auto result = connection.Query(fmt::format(
      "SELECT json_keys(alignedNucleotideSequences), json_keys(alignedAminoAcidSequences), "
      "json_keys(unalignedNucleotideSequences), json_keys(nucleotideInsertions), "
      "json_keys(aminoAcidInsertions) "
      "FROM '{}' LIMIT 1; ",
      input_filename.string()
   ));
   if (result->HasError()) {
      throw silo::preprocessing::PreprocessingException(
         "Preprocessing exception when retrieving the fields 'alignedNucleotideSequences' "
         "and 'alignedAminoAcidSequences', duckdb threw with error: " +
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

   const auto& nuc_sequence_names_to_validate = extractStringListValue(*result, 0, 0);
   validateStruct(
      nuc_sequence_names_to_validate,
      nuc_sequence_names,
      "aligned nucleotide sequences",
      input_filename
   );
   const auto& aa_sequence_names_to_validate = extractStringListValue(*result, 0, 1);
   validateStruct(
      aa_sequence_names_to_validate,
      aa_sequence_names,
      "aligned amino acid sequences",
      input_filename
   );
   const auto& unaligned_nuc_sequence_names_to_validate = extractStringListValue(*result, 0, 2);
   validateStruct(
      unaligned_nuc_sequence_names_to_validate,
      nuc_sequence_names,
      "unaligned nucleotide sequences",
      input_filename
   );
   const auto& nuc_insertion_names_to_validate = extractStringListValue(*result, 0, 3);
   validateStruct(
      nuc_insertion_names_to_validate, nuc_sequence_names, "nucleotide insertions", input_filename
   );
   const auto& aa_insertion_names_to_validate = extractStringListValue(*result, 0, 4);
   validateStruct(
      aa_insertion_names_to_validate, aa_sequence_names, "amino acid insertions", input_filename
   );
}

}  // namespace silo::preprocessing
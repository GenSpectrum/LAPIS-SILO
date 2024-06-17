#include "silo/preprocessing/sequence_info.h"

#include <duckdb.hpp>

#include "silo/preprocessing/metadata_info.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"

namespace silo::preprocessing {

std::vector<std::string> SequenceInfo::getAlignedSequenceSelects(
   const silo::ReferenceGenomes& reference_genomes,
   const PreprocessingDatabase& preprocessing_db
) {
   std::vector<std::string> sequence_selects;
   sequence_selects.reserve(
      reference_genomes.nucleotide_sequences.size() + reference_genomes.aa_sequences.size()
   );
   for (const auto& [name, _] : reference_genomes.nucleotide_sequences) {
      sequence_selects.emplace_back(getNucleotideSequenceSelect(name, preprocessing_db));
   }
   for (const auto& [name, _] : reference_genomes.aa_sequences) {
      sequence_selects.emplace_back(getAminoAcidSequenceSelect(name, preprocessing_db));
   }
   return sequence_selects;
}

std::string SequenceInfo::getNucleotideSequenceSelect(
   std::string_view seq_name,
   const PreprocessingDatabase& preprocessing_db
) {
   const std::string column_name_in_data = fmt::format("nuc_{}", seq_name);

   return fmt::format(
      "{0} AS nuc_{1}",
      preprocessing_db.compress_nucleotide_functions.at(seq_name)->generateSqlStatement(
         column_name_in_data
      ),
      seq_name
   );
}

std::string SequenceInfo::getUnalignedSequenceSelect(
   std::string_view seq_name,
   const PreprocessingDatabase& preprocessing_db
) {
   const std::string column_name_in_data = fmt::format("unaligned_nuc_{}", seq_name);
   return fmt::format(
      "{0} AS unaligned_nuc_{1}",
      preprocessing_db.compress_nucleotide_functions.at(seq_name)->generateSqlStatement(
         column_name_in_data
      ),
      seq_name
   );
}

std::string SequenceInfo::getAminoAcidSequenceSelect(
   std::string_view seq_name,
   const PreprocessingDatabase& preprocessing_db
) {
   const std::string column_name_in_data = fmt::format("aa_{}", seq_name);

   return fmt::format(
      "{0} AS aa_{1}",
      preprocessing_db.compress_amino_acid_functions.at(seq_name)->generateSqlStatement(
         column_name_in_data
      ),
      seq_name
   );
}

namespace {
void validateStruct(
   std::vector<std::string> names_to_validate,
   std::vector<std::string> names_to_validate_against,
   std::string name_type,
   const std::filesystem::path& input_filename
) {
   for (const std::string& name : names_to_validate) {
      if (std::find(names_to_validate_against.begin(), names_to_validate_against.end(), name) ==
          names_to_validate_against.end()) {
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
      if (std::find(names_to_validate.begin(), names_to_validate.end(), name) == names_to_validate.end()) {
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
   duckdb::Connection& connection,
   const std::filesystem::path& input_filename
) {
   if (MetadataInfo::isNdjsonFileEmpty(input_filename)) {
      return;
   }

   const std::vector<std::string> nuc_sequence_names =
      reference_genomes.getSequenceNames<Nucleotide>();
   const std::vector<std::string> aa_sequence_names =
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

   auto nuc_sequence_names_to_validate = extractStringListValue(*result, 0, 0);
   validateStruct(
      nuc_sequence_names_to_validate,
      nuc_sequence_names,
      "aligned nucleotide sequences",
      input_filename
   );
   auto aa_sequence_names_to_validate = extractStringListValue(*result, 0, 1);
   validateStruct(
      aa_sequence_names_to_validate,
      aa_sequence_names,
      "aligned amino acid sequences",
      input_filename
   );
   auto unaligned_nuc_sequence_names_to_validate = extractStringListValue(*result, 0, 2);
   validateStruct(
      unaligned_nuc_sequence_names_to_validate,
      nuc_sequence_names,
      "unaligned nucleotide sequences",
      input_filename
   );
   auto nuc_insertion_names_to_validate = extractStringListValue(*result, 0, 3);
   validateStruct(
      nuc_insertion_names_to_validate, nuc_sequence_names, "nucleotide insertions", input_filename
   );
   auto aa_insertion_names_to_validate = extractStringListValue(*result, 0, 4);
   validateStruct(
      aa_insertion_names_to_validate, aa_sequence_names, "amino acid insertions", input_filename
   );
}

}  // namespace silo::preprocessing
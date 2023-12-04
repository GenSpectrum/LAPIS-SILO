#include "silo/preprocessing/sequence_info.h"

#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/reference_genomes.h"

namespace {
std::vector<std::string> extractStringListValue(
   duckdb::MaterializedQueryResult& result,
   size_t row,
   size_t column
) {
   std::vector<std::string> return_value;
   const duckdb::Value tmp_value = result.GetValue(column, row);
   std::vector<duckdb::Value> child_values = duckdb::ListValue::GetChildren(tmp_value);
   std::transform(
      child_values.begin(),
      child_values.end(),
      std::back_inserter(return_value),
      [](const duckdb::Value& value) { return value.GetValue<std::string>(); }
   );
   return return_value;
}
}  // namespace

namespace silo::preprocessing {

SequenceInfo::SequenceInfo(const silo::ReferenceGenomes& reference_genomes) {
   for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
      nuc_sequence_names.push_back(name);
   }
   for (const auto& [name, sequence] : reference_genomes.raw_aa_sequences) {
      aa_sequence_names.push_back(name);
   }
}

std::vector<std::string>& SequenceInfo::getNucSequenceNames() {
   return nuc_sequence_names;
}

std::vector<std::string>& SequenceInfo::getAASequenceNames() {
   return aa_sequence_names;
}

std::vector<std::string> SequenceInfo::getSequenceSelects() {
   std::vector<std::string> sequence_selects;
   for (const std::string& name : nuc_sequence_names) {
      sequence_selects.emplace_back(fmt::format(
         "compressNuc(alignedNucleotideSequences.{0}, "
         "'{0}') as nuc_{0}",
         name
      ));
   }
   for (const std::string& name : aa_sequence_names) {
      sequence_selects.emplace_back(fmt::format(
         "compressAA(alignedAminoAcidSequences.{0}, "
         "'{0}') as gene_{0}",
         name
      ));
   }
   return sequence_selects;
}

std::string SequenceInfo::getNucInsertionSelect() {
   if (nuc_sequence_names.empty()) {
      return "''";
   }
   if (nuc_sequence_names.size() == 1) {
      return "list_string_agg(nucleotideInsertions." + nuc_sequence_names.at(0) + ")";
   }

   std::vector<std::string> list_transforms;
   for (const std::string& sequence_name : nuc_sequence_names) {
      list_transforms.push_back(
         fmt::format("list_transform(nucleotideInsertions.{0}, x ->'{0}:' || x)", sequence_name)
      );
   }

   return "list_string_agg(flatten([" + boost::join(list_transforms, ",") + "]))";
}

std::string SequenceInfo::getAAInsertionSelect() {
   if (aa_sequence_names.empty()) {
      return "''";
   }
   if (aa_sequence_names.size() == 1) {
      return fmt::format(
         "list_string_agg(list_transform(aminoAcidInsertions.{0}, x ->'{0}:' || x))",
         aa_sequence_names.at(0)
      );
   }

   std::vector<std::string> list_transforms;
   for (const std::string& sequence_name : aa_sequence_names) {
      list_transforms.push_back(
         fmt::format("list_transform(aminoAcidInsertions.{0}, x ->'{0}:' || x)", sequence_name)
      );
   }

   return "list_string_agg(flatten([" + boost::join(list_transforms, ",") + "]))";
}

void SequenceInfo::validate(duckdb::Connection& connection, std::string_view input_filename) const {
   auto result = connection.Query(fmt::format(
      "SELECT json_keys(alignedNucleotideSequences), json_keys(alignedAminoAcidSequences) "
      "FROM "
      "'{}' LIMIT 1; ",
      input_filename
   ));
   if (result->HasError()) {
      throw silo::preprocessing::PreprocessingException(
         "Preprocessing exception when retrieving the fields 'alignedNucleotideSequences' and "
         "'alignedAminoAcidSequences', duckdb threw with error: " +
         result->GetError()
      );
   }
   if (result->RowCount() == 0) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("File {} is empty, which must not be empty at this point", input_filename)
      );
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
            input_filename
         ));
      }
   }
   for (const std::string& name : nuc_sequence_names) {
      if (std::find(nuc_sequence_names_to_validate.begin(), nuc_sequence_names_to_validate.end(), name)
          == nuc_sequence_names_to_validate.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aligned nucleotide sequence {} which is contained in the reference sequences is "
            "not contained in the input file {}.",
            name,
            input_filename
         ));
      }
   }
   for (const std::string& name : aa_sequence_names_to_validate) {
      if (std::find(aa_sequence_names.begin(), aa_sequence_names.end(), name) == aa_sequence_names.end()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The aligned amino acid sequence {} which is contained in the input file {} is "
            "not contained in the reference sequences.",
            name,
            input_filename
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
            input_filename
         ));
      }
   }
}

}  // namespace silo::preprocessing
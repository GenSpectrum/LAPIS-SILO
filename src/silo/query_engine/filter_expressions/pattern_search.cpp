#include "silo/query_engine/filter_expressions/pattern_search.h"

#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/operators/bitmap_producer.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/union.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

PatternSearch::PatternSearch(
   std::optional<std::string> nuc_sequence_name,
   uint32_t position,
   std::vector<NUCLEOTIDE_SYMBOL> pattern
)
    : nuc_sequence_name(std::move(nuc_sequence_name)),
      position(position),
      pattern(std::move(pattern)) {}

std::string PatternSearch::toString(const silo::Database& /*database*/) const {
   const std::string nuc_sequence_name_prefix =
      nuc_sequence_name ? nuc_sequence_name.value() + ":" : "";
   std::vector<char> pattern_chars(pattern.size());
   std::transform(pattern.begin(), pattern.end(), pattern_chars.begin(), nucleotideSymbolToChar);
   return nuc_sequence_name_prefix + std::to_string(position + 1) +
          std::string(pattern_chars.data(), pattern_chars.size());
}

std::unique_ptr<silo::query_engine::operators::Operator> PatternSearch::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   const std::string nuc_sequence_name_or_default =
      nuc_sequence_name.value_or(database.database_config.default_nucleotide_sequence);
   CHECK_SILO_QUERY(
      database.nuc_sequences.contains(nuc_sequence_name_or_default),
      "Database does not contain the nucleotide sequence with name: '" +
         nuc_sequence_name_or_default + "'"
   )

   const auto& seq_store_partition =
      database_partition.nuc_sequences.at(nuc_sequence_name_or_default);
   const auto genome_length = seq_store_partition.reference_genome.size();
   std::vector<std::unique_ptr<operators::Operator>> result;
   const bool pattern_contains_unfiltered_symbol =
      std::any_of(pattern.begin(), pattern.end(), [](auto pos) {
         return pos == NUCLEOTIDE_SYMBOL::N || pos == NUCLEOTIDE_SYMBOL::GAP;
      });
   for (uint32_t genome_start_pos = position; genome_start_pos <= genome_length - pattern.size();
        ++genome_start_pos) {
      uint32_t mutation_count = 0;
      for (uint32_t pattern_pos = 0; pattern_pos < pattern.size(); ++pattern_pos) {
         mutation_count += static_cast<uint32_t>(
            seq_store_partition.reference_genome[genome_start_pos + pattern_pos] !=
            pattern[pattern_pos]
         );
      }
      const auto range = std::make_pair(genome_start_pos, genome_start_pos + pattern.size());
      auto filter = seq_store_partition.mutation_filter.filter(range, mutation_count);
      if (filter.has_value() && !pattern_contains_unfiltered_symbol) {
         std::vector<uint32_t> matching_genome_ids;
         for (auto genome_id : *filter.value()) {
            bool match = true;
            for (uint32_t pattern_pos = 0; match && pattern_pos < pattern.size(); ++pattern_pos) {
               auto genome_pos = genome_start_pos + pattern_pos;
               auto& pattern_symbol = pattern[pattern_pos];
               const auto* bitmap = seq_store_partition.getBitmap(genome_pos, pattern_symbol);
               if (pattern_symbol == seq_store_partition.reference_genome[genome_pos]) {
                  match &= !bitmap->contains(genome_id);
               } else {
                  match &= bitmap->contains(genome_id);
               }
            }
            if (match) {
               matching_genome_ids.push_back(genome_id);
            }
         }
         if (!matching_genome_ids.empty()) {
            result.push_back(std::make_unique<operators::BitmapProducer>(
               [&]() {
                  auto matching_bitmap = std::make_unique<roaring::Roaring>(
                     matching_genome_ids.size(), matching_genome_ids.data()
                  );
                  return OperatorResult(matching_bitmap.release());
               },
               database_partition.sequenceCount
            ));
         }

      } else {
         std::vector<std::unique_ptr<operators::Operator>> negated_matches;
         std::vector<std::unique_ptr<operators::Operator>> matches;
         for (uint32_t pattern_pos = 0; pattern_pos < pattern.size(); ++pattern_pos) {
            auto genome_pos = genome_start_pos + pattern_pos;
            auto& pattern_symbol = pattern[pattern_pos];
            if (pattern_symbol == seq_store_partition.reference_genome[genome_pos]) {
               negated_matches.push_back(std::make_unique<operators::IndexScan>(
                  seq_store_partition.getBitmap(genome_pos, pattern_symbol),
                  seq_store_partition.sequence_count
               ));
            } else {
               matches.push_back(std::make_unique<operators::IndexScan>(
                  seq_store_partition.getBitmap(genome_pos, pattern_symbol),
                  seq_store_partition.sequence_count
               ));
            }
         }
         if (matches.empty()) {
            const operators::Union union_operator{
               std::move(negated_matches), seq_store_partition.sequence_count};
            result.push_back(union_operator.negate());
         } else {
            result.push_back(std::make_unique<operators::Intersection>(
               std::move(matches), std::move(negated_matches), seq_store_partition.sequence_count
            ));
         }
      }
   }
   if (!result.empty()) {
      return std::make_unique<operators::Union>(
         std::move(result), seq_store_partition.sequence_count
      );
   }

   return std::make_unique<operators::Empty>(database_partition.sequenceCount);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PatternSearch>& filter) {
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("position"),
      "The field 'position' is required in a PatternSearch expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned() && json["position"].get<uint32_t>() > 0,
      "The field 'position' in a PatternSearch expression needs to be an unsigned "
      "integer greater than 0"
   )
   CHECK_SILO_QUERY(
      json.contains("pattern") && json["pattern"].is_string(),
      "The string field 'pattern' is required in a PatternSearch expression"
   )
   std::optional<std::string> nuc_sequence_name;
   if (json.contains("sequenceName")) {
      nuc_sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   const std::string pattern_chars = json["pattern"].get<std::string>();

   std::vector<NUCLEOTIDE_SYMBOL> pattern(pattern_chars.size());
   std::transform(
      pattern_chars.begin(),
      pattern_chars.end(),
      pattern.begin(),
      [&pattern_chars](char character) {
         auto symbol = charToNucleotideSymbol(character);
         CHECK_SILO_QUERY(
            symbol.has_value(),
            "The pattern " + pattern_chars + " contains the illegal character " +
               std::to_string(character)
         )
         return *symbol;
      }
   );

   filter = std::make_unique<PatternSearch>(nuc_sequence_name, position, pattern);
}

}  // namespace silo::query_engine::filter_expressions

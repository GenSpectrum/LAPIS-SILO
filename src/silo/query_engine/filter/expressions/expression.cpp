#include "silo/query_engine/filter/expressions/expression.h"

#include <string>

#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/bool_equals.h"
#include "silo/query_engine/filter/expressions/date_between.h"
#include "silo/query_engine/filter/expressions/date_equals.h"
#include "silo/query_engine/filter/expressions/exact.h"
#include "silo/query_engine/filter/expressions/false.h"
#include "silo/query_engine/filter/expressions/float_between.h"
#include "silo/query_engine/filter/expressions/float_equals.h"
#include "silo/query_engine/filter/expressions/has_mutation.h"
#include "silo/query_engine/filter/expressions/insertion_contains.h"
#include "silo/query_engine/filter/expressions/int_between.h"
#include "silo/query_engine/filter/expressions/int_equals.h"
#include "silo/query_engine/filter/expressions/is_null.h"
#include "silo/query_engine/filter/expressions/lineage_filter.h"
#include "silo/query_engine/filter/expressions/maybe.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/nof.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/phylo_child_filter.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/string_search.h"
#include "silo/query_engine/filter/expressions/symbol_equals.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::filter::expressions {

Expression::Expression() = default;

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode) {
   if (mode == Expression::UPPER_BOUND) {
      return Expression::LOWER_BOUND;
   }
   if (mode == Expression::LOWER_BOUND) {
      return Expression::UPPER_BOUND;
   }
   return mode;
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
void from_json(const nlohmann::json& json, std::unique_ptr<Expression>& filter) {
   CHECK_SILO_QUERY(json.contains("type"), "The field 'type' is required in any filter expression");
   CHECK_SILO_QUERY(
      json["type"].is_string(),
      "The field 'type' in all filter expressions needs to be a string, but is: {}",
      json["type"].dump()
   );
   const std::string expression_type = json["type"];
   if (expression_type == "True") {
      filter = json.get<std::unique_ptr<True>>();
   } else if (expression_type == "False") {
      filter = json.get<std::unique_ptr<False>>();
   } else if (expression_type == "And") {
      filter = json.get<std::unique_ptr<And>>();
   } else if (expression_type == "Or") {
      filter = json.get<std::unique_ptr<Or>>();
   } else if (expression_type == "N-Of") {
      filter = json.get<std::unique_ptr<NOf>>();
   } else if (expression_type == "Not") {
      filter = json.get<std::unique_ptr<Negation>>();
   } else if (expression_type == "DateBetween") {
      filter = json.get<std::unique_ptr<DateBetween>>();
   } else if (expression_type == "DateEquals") {
      filter = json.get<std::unique_ptr<DateEquals>>();
   } else if (expression_type == "NucleotideEquals") {
      filter = json.get<std::unique_ptr<SymbolEquals<Nucleotide>>>();
   } else if (expression_type == "HasNucleotideMutation") {
      filter = json.get<std::unique_ptr<HasMutation<Nucleotide>>>();
   } else if (expression_type == "AminoAcidEquals") {
      filter = json.get<std::unique_ptr<SymbolEquals<AminoAcid>>>();
   } else if (expression_type == "HasAminoAcidMutation") {
      filter = json.get<std::unique_ptr<HasMutation<AminoAcid>>>();
   } else if (expression_type == "Lineage") {
      filter = json.get<std::unique_ptr<LineageFilter>>();
   } else if (expression_type == "PhyloDescendantOf") {
      filter = json.get<std::unique_ptr<PhyloChildFilter>>();
   } else if (expression_type == "StringEquals") {
      filter = json.get<std::unique_ptr<StringEquals>>();
   } else if (expression_type == "StringInSet") {
      filter = json.get<std::unique_ptr<StringInSet>>();
   } else if (expression_type == "StringSearch") {
      filter = json.get<std::unique_ptr<StringSearch>>();
   } else if (expression_type == "BooleanEquals") {
      filter = json.get<std::unique_ptr<BoolEquals>>();
   } else if (expression_type == "IntEquals") {
      filter = json.get<std::unique_ptr<IntEquals>>();
   } else if (expression_type == "IntBetween") {
      filter = json.get<std::unique_ptr<IntBetween>>();
   } else if (expression_type == "FloatEquals") {
      filter = json.get<std::unique_ptr<FloatEquals>>();
   } else if (expression_type == "FloatBetween") {
      filter = json.get<std::unique_ptr<FloatBetween>>();
   } else if (expression_type == "Maybe") {
      filter = json.get<std::unique_ptr<Maybe>>();
   } else if (expression_type == "Exact") {
      filter = json.get<std::unique_ptr<Exact>>();
   } else if (expression_type == "InsertionContains") {
      filter = json.get<std::unique_ptr<InsertionContains<Nucleotide>>>();
   } else if (expression_type == "AminoAcidInsertionContains") {
      filter = json.get<std::unique_ptr<InsertionContains<AminoAcid>>>();
   } else if (expression_type == "IsNull") {
      filter = json.get<std::unique_ptr<IsNull>>();
   } else if (expression_type == "IsNotNull") {
      filter = std::make_unique<Negation>(json.get<std::unique_ptr<IsNull>>());
   } else {
      throw query_engine::IllegalQueryException(
         "Unknown object filter type '" + expression_type + "'"
      );
   }
}

}  // namespace silo::query_engine::filter::expressions

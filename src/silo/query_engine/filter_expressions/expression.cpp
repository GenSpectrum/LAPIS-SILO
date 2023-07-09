#include "silo/query_engine/filter_expressions/expression.h"

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "silo/query_engine/filter_expressions/aa_symbol_equals.h"
#include "silo/query_engine/filter_expressions/and.h"
#include "silo/query_engine/filter_expressions/date_between.h"
#include "silo/query_engine/filter_expressions/exact.h"
#include "silo/query_engine/filter_expressions/false.h"
#include "silo/query_engine/filter_expressions/float_between.h"
#include "silo/query_engine/filter_expressions/float_equals.h"
#include "silo/query_engine/filter_expressions/has_aa_mutation.h"
#include "silo/query_engine/filter_expressions/has_mutation.h"
#include "silo/query_engine/filter_expressions/int_between.h"
#include "silo/query_engine/filter_expressions/int_equals.h"
#include "silo/query_engine/filter_expressions/maybe.h"
#include "silo/query_engine/filter_expressions/negation.h"
#include "silo/query_engine/filter_expressions/nof.h"
#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/filter_expressions/pango_lineage_filter.h"
#include "silo/query_engine/filter_expressions/string_equals.h"
#include "silo/query_engine/filter_expressions/true.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo::query_engine::filter_expressions {

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

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Expression>& filter) {
   CHECK_SILO_QUERY(json.contains("type"), "The field 'type' is required in any filter expression")
   CHECK_SILO_QUERY(
      json["type"].is_string(),
      "The field 'type' in all filter expressions needs to be a string, but is: " +
         json["type"].dump()
   )
   const std::string expression_type = json["type"];
   if (expression_type == "True") {
      filter = json.get<std::unique_ptr<silo::query_engine::filter_expressions::True>>();
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
   } else if (expression_type == "NucleotideEquals") {
      filter = json.get<std::unique_ptr<NucleotideSymbolEquals>>();
   } else if (expression_type == "HasNucleotideMutation") {
      filter = json.get<std::unique_ptr<HasMutation>>();
   } else if (expression_type == "AminoAcidEquals") {
      filter = json.get<std::unique_ptr<AASymbolEquals>>();
   } else if (expression_type == "HasAminoAcidMutation") {
      filter = json.get<std::unique_ptr<HasAAMutation>>();
   } else if (expression_type == "PangoLineage") {
      filter = json.get<std::unique_ptr<PangoLineageFilter>>();
   } else if (expression_type == "StringEquals") {
      filter = json.get<std::unique_ptr<StringEquals>>();
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
   } else {
      throw QueryParseException("Unknown object filter type '" + expression_type + "'");
   }
}

}  // namespace silo::query_engine::filter_expressions

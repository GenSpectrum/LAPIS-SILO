#include "silo/query_engine/parser.h"

#include <istream>
#include <memory>
#include <sstream>

// query_parse_exception.h must be before the RAPIDJSON_ASSERT because it is used there
#include "silo/query_engine/query_parse_exception.h"
// Do not remove the next line. It overwrites the rapidjson abort, so it can throw an exception and
// does not abort.
#define RAPIDJSON_ASSERT(x)                                                    \
   if (!(x))                                                                   \
   throw silo::QueryParseException(                                            \
      "The query was not a valid JSON: " + std::string(RAPIDJSON_STRINGIFY(x)) \
   )
#include <rapidjson/document.h>
#include <silo/query_engine/filter_expressions/has_mutation.h>

#include "silo/database.h"
#include "silo/query_engine/filter_expressions/and.h"
#include "silo/query_engine/filter_expressions/date_between.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/filter_expressions/false.h"
#include "silo/query_engine/filter_expressions/negation.h"
#include "silo/query_engine/filter_expressions/nof.h"
#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"
#include "silo/query_engine/filter_expressions/nucleotide_symbol_maybe.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/filter_expressions/pango_lineage.h"
#include "silo/query_engine/filter_expressions/string_equals.h"
#include "silo/query_engine/filter_expressions/true.h"

#define CHECK_SILO_QUERY(condition, message)    \
   if (!(condition)) {                          \
      throw silo::QueryParseException(message); \
   }

namespace filters = silo::query_engine::filter_expressions;

namespace silo::query_engine {

// NOLINTNEXTLINE(readability-function-cognitive-complexity, misc-no-recursion)
std::unique_ptr<filters::Expression> parseExpression(
   const Database& database,
   const rapidjson::Value& json_value,
   int exact_maybe  // 1 = exact, -1 = maybe, 0 = standard 2-ary logic
) {
   CHECK_SILO_QUERY(
      json_value.HasMember("type"), "The field 'type' is required in 'filterExpression'"
   )
   CHECK_SILO_QUERY(
      json_value["type"].IsString(), "The field 'type' in 'filterExpression' needs to be a string"
   )
   const std::string expression_type = json_value["type"].GetString();
   if (expression_type == "True") {
      return std::make_unique<filters::True>();
   }
   if (expression_type == "False") {
      return std::make_unique<filters::False>();
   }
   if (expression_type == "And") {
      std::vector<std::unique_ptr<filters::Expression>> children;
      CHECK_SILO_QUERY(
         json_value.HasMember("children"), "The field 'children' is required in an And expression"
      )
      CHECK_SILO_QUERY(
         json_value["children"].IsArray(),
         "The field 'children' in an And expression needs to be an array"
      )
      std::transform(
         json_value["children"].GetArray().begin(),
         json_value["children"].GetArray().end(),
         std::back_inserter(children),
         [&](const rapidjson::Value& value) {  // NOLINT(misc-no-recursion)
            return parseExpression(database, value, exact_maybe);
         }
      );
      return std::make_unique<filters::And>(std::move(children));
   }
   if (expression_type == "Or") {
      std::vector<std::unique_ptr<filters::Expression>> children;
      CHECK_SILO_QUERY(
         json_value.HasMember("children"), "The field 'children' is required in an Or expression"
      )
      CHECK_SILO_QUERY(
         json_value["children"].IsArray(),
         "The field 'children' in an Or expression needs to be an array"
      )
      std::transform(
         json_value["children"].GetArray().begin(),
         json_value["children"].GetArray().end(),
         std::back_inserter(children),
         [&](const rapidjson::Value& value) {  // NOLINT(misc-no-recursion)
            return parseExpression(database, value, exact_maybe);
         }
      );
      return std::make_unique<filters::Or>(std::move(children));
   }
   if (expression_type == "N-Of") {
      CHECK_SILO_QUERY(
         json_value.HasMember("children"), "The field 'children' is required in an N-Of expression"
      )
      CHECK_SILO_QUERY(
         json_value["children"].IsArray(),
         "The field 'children' in an N-Of expression needs to be an array"
      )
      CHECK_SILO_QUERY(
         json_value.HasMember("numberOfMatchers"),
         "The field 'numberOfMatchers' is required in an N-Of expression"
      )
      CHECK_SILO_QUERY(
         json_value["numberOfMatchers"].IsUint(),
         "The field 'numberOfMatchers' in an N-Of expression needs to be an unsigned integer"
      )

      const unsigned number_of_matchers = json_value["numberOfMatchers"].GetUint();
      const bool match_exactly = json_value["matchExactly"].GetBool();

      std::vector<std::unique_ptr<filters::Expression>> children;
      std::transform(
         json_value["children"].GetArray().begin(),
         json_value["children"].GetArray().end(),
         std::back_inserter(children),
         [&](const rapidjson::Value& json_value) {  // NOLINT(misc-no-recursion)
            return parseExpression(database, json_value, exact_maybe);
         }
      );
      return std::make_unique<filters::NOf>(std::move(children), number_of_matchers, match_exactly);
   }
   if (expression_type == "Not") {
      auto child = parseExpression(database, json_value["child"], -exact_maybe);
      return std::make_unique<filters::Negation>(std::move(child));
   }
   if (expression_type == "DateBetween") {
      const std::string& column = json_value["column"].GetString();

      std::optional<time_t> date_from;
      if (!json_value["from"].IsNull()) {
         struct std::tm time_object {};
         std::istringstream date_from_stream(json_value["from"].GetString());
         date_from_stream >> std::get_time(&time_object, "%Y-%m-%d");
         date_from = mktime(&time_object);
      }

      std::optional<time_t> date_to;
      if (!json_value["to"].IsNull()) {
         struct std::tm time_object {};
         std::istringstream date_to_stream(json_value["to"].GetString());
         date_to_stream >> std::get_time(&time_object, "%Y-%m-%d");
         date_to = mktime(&time_object);
      }
      return std::make_unique<filters::DateBetween>(column, date_from, date_to);
   }
   if (expression_type == "NucleotideEquals") {
      CHECK_SILO_QUERY(
         json_value.HasMember("position"),
         "The field 'position' is required in a NucleotideEquals expression"
      )
      CHECK_SILO_QUERY(
         json_value["position"].GetUint(),
         "The field 'position' in a NucleotideEquals expression needs to be an unsigned "
         "integer"
      )
      const unsigned position = json_value["position"].GetUint() - 1;
      const std::string& nucleotide_symbol = json_value["symbol"].GetString();
      NUCLEOTIDE_SYMBOL value;
      if (nucleotide_symbol.at(0) == '.') {
         const char character = database.global_reference[0].at(position);
         value = toNucleotideSymbol(character);
      } else {
         value = toNucleotideSymbol(nucleotide_symbol.at(0));
      }
      if (exact_maybe >= 0) {
         return std::make_unique<filters::NucleotideSymbolEquals>(position, value);
      }
      return std::make_unique<filters::NucleotideSymbolMaybe>(position, value);
   }
   if (expression_type == "HasNucleotideMutation") {
      CHECK_SILO_QUERY(
         json_value.HasMember("position"),
         "The field 'position' is required in a HasNucleotideMutation expression"
      )
      CHECK_SILO_QUERY(
         json_value["position"].GetUint(),
         "The field 'position' in a HasNucleotideMutation expression needs to be an unsigned "
         "integer"
      )
      const unsigned position = json_value["position"].GetUint() - 1;
      if (exact_maybe >= 0) {
         std::make_unique<filters::HasMutation>(position);
      }
      const char ref_symbol = database.global_reference[0].at(position);
      return std::make_unique<filters::Negation>(std::make_unique<filters::NucleotideSymbolEquals>(
         position, silo::toNucleotideSymbol(ref_symbol)
      ));
   }
   if (expression_type == "PangoLineage") {
      CHECK_SILO_QUERY(
         json_value.HasMember("includeSublineages"),
         "The field 'includeSublineages' is required in a PangoLineage expression"
      )
      const bool include_sublineages = json_value["includeSublineages"].GetBool();
      CHECK_SILO_QUERY(
         json_value.HasMember("value"), "The field 'value' is required in a PangoLineage expression"
      )
      const std::string& lineage = json_value["value"].GetString();
      return std::make_unique<filters::PangoLineage>(lineage, include_sublineages);
   }
   if (expression_type == "StringEquals") {
      const std::string& column = json_value["column"].GetString();
      const std::string& value = json_value["value"].GetString();
      return std::make_unique<filters::StringEquals>(column, value);
   }
   if (expression_type == "Maybe") {
      return parseExpression(database, json_value["child"], -1);
   }
   if (expression_type == "Exact") {
      return parseExpression(database, json_value["child"], 1);
   }
   throw QueryParseException("Unknown object filter type '" + expression_type + "'");
}

}  // namespace silo::query_engine
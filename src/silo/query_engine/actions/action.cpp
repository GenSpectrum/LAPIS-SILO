#include "silo/query_engine/actions/action.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <map>
#include <memory>
#include <variant>

#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/aa_mutations.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/nuc_mutations.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

Action::Action() = default;

std::string stringToLowerCase(std::string str) {
   std::transform(str.begin(), str.end(), str.begin(), [](unsigned char character) {
      return std::tolower(character);
   });
   return str;
}

void Action::applyOrderByAndLimit(QueryResult& result) const {
   auto& result_vector = result.query_result;
   auto cmp = [&](const QueryResultEntry& entry1, const QueryResultEntry& entry2) {
      for (const OrderByField& field : order_by_fields) {
         if (entry1.fields.at(field.name) == entry2.fields.at(field.name)) {
            continue;
         }
         return entry1.fields.at(field.name) < entry2.fields.at(field.name) ? field.ascending
                                                                            : !field.ascending;
      }
      return false;
   };
   size_t end_of_sort = std::min(
      static_cast<size_t>(limit.value_or(result_vector.size()) + offset.value_or(0UL)),
      result_vector.size()
   );
   if (end_of_sort < result_vector.size()) {
      std::partial_sort(
         result_vector.begin(),
         result_vector.begin() + static_cast<int64_t>(end_of_sort),
         result_vector.end(),
         cmp
      );
   } else {
      std::sort(result_vector.begin(), result_vector.end(), cmp);
   }

   if (offset.has_value() && offset.value() > 0) {
      if (offset.value() >= result_vector.size()) {
         result_vector = {};
         return;
      }
      auto begin = result_vector.begin() + offset.value();
      auto end = end_of_sort < result_vector.size()
                    ? result_vector.begin() + static_cast<int64_t>(end_of_sort)
                    : result_vector.end();
      std::copy(begin, end, result_vector.begin());
      end_of_sort -= offset.value();
   }
   if (end_of_sort < result_vector.size()) {
      result_vector.resize(end_of_sort);
   }
}

void Action::setOrdering(
   const std::vector<OrderByField>& order_by_fields_,
   std::optional<uint32_t> limit_,
   std::optional<uint32_t> offset_
) {
   order_by_fields = order_by_fields_;
   limit = limit_;
   offset = offset_;
}

QueryResult Action::executeAndOrder(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   QueryResult result = execute(database, std::move(bitmap_filter));
   applyOrderByAndLimit(result);
   return result;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, Action::OrderByField& field) {
   if (json.is_string()) {
      field = {json.get<std::string>(), true};
      return;
   }
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("field") && json.contains("order") &&
         json["field"].is_string() && json["order"].is_string(),
      "The orderByField '" + json.dump() +
         "' must be either a string or an object containing the fields 'field':string and "
         "'order':string, where the value of order is 'ascending' or 'descending'"
   )
   field = {json["field"].get<std::string>(), json["order"].get<std::string>() == "ascending"};
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action) {
   CHECK_SILO_QUERY(json.contains("type"), "The field 'type' is required in any action")
   CHECK_SILO_QUERY(
      json["type"].is_string(),
      "The field 'type' in all actions needs to be a string, but is: " + json["type"].dump()
   )
   const std::string expression_type = json["type"];
   if (expression_type == "Aggregated") {
      action = json.get<std::unique_ptr<Aggregated>>();
   } else if (expression_type == "Mutations") {
      action = json.get<std::unique_ptr<NucMutations>>();
   } else if (expression_type == "Details") {
      action = json.get<std::unique_ptr<Details>>();
   } else if (expression_type == "AminoAcidMutations") {
      action = json.get<std::unique_ptr<AAMutations>>();
   } else if (expression_type == "Fasta") {
      action = json.get<std::unique_ptr<Fasta>>();
   } else if (expression_type == "FastaAligned") {
      action = json.get<std::unique_ptr<FastaAligned>>();
   } else {
      throw QueryParseException(expression_type + " is not a valid action");
   }

   auto order_by_fields = json.contains("orderByFields")
                             ? json["orderByFields"].get<std::vector<Action::OrderByField>>()
                             : std::vector<Action::OrderByField>();
   CHECK_SILO_QUERY(
      !json.contains("limit") || json["limit"].is_number_unsigned(),
      "If the action contains a limit, it must be a non-negative number"
   )
   CHECK_SILO_QUERY(
      !json.contains("offset") || json["offset"].is_number_unsigned(),
      "If the action contains an offset, it must be a non-negative number"
   )
   auto limit = json.contains("limit") ? std::optional<uint32_t>(json["limit"].get<uint32_t>())
                                       : std::nullopt;
   auto offset = json.contains("offset") ? std::optional<uint32_t>(json["offset"].get<uint32_t>())
                                         : std::nullopt;
   action->setOrdering(order_by_fields, limit, offset);
}

}  // namespace silo::query_engine::actions

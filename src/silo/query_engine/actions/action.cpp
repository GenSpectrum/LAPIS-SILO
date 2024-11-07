#include "silo/query_engine/actions/action.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <map>
#include <memory>
#include <random>
#include <utility>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/insertions.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

Action::Action() = default;

void Action::applySort(QueryResult& result) const {
   auto& result_vector = result.entriesMut();

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
   const size_t end_of_sort = std::min(
      static_cast<size_t>(limit.value_or(result_vector.size()) + offset.value_or(0UL)),
      result_vector.size()
   );
   if (randomize_seed) {
      std::default_random_engine rng(*randomize_seed);
      std::shuffle(
         result_vector.begin(), result_vector.begin() + static_cast<int64_t>(end_of_sort), rng
      );
   }
   if (!order_by_fields.empty()) {
      if (end_of_sort < result_vector.size()) {
         std::partial_sort(
            result_vector.begin(),
            result_vector.begin() + static_cast<int64_t>(end_of_sort),
            result_vector.end(),
            cmp
         );
      } else {
         std::ranges::sort(result_vector, cmp);
      }
   }
}

void Action::applyOffsetAndLimit(QueryResult& result) const {
   auto& result_vector = result.entriesMut();

   size_t end_of_sort = std::min(
      static_cast<size_t>(limit.value_or(result_vector.size()) + offset.value_or(0UL)),
      result_vector.size()
   );

   if (offset.has_value() && offset.value() >= end_of_sort) {
      result.clear();
      return;
   }

   if (offset.has_value() && offset.value() > 0) {
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
   std::optional<uint32_t> offset_,
   std::optional<uint32_t> randomize_seed_
) {
   order_by_fields = order_by_fields_;
   limit = limit_;
   offset = offset_;
   randomize_seed = randomize_seed_;
}

static const size_t MATERIALIZATION_CUTOFF = 10000;

QueryResult Action::executeAndOrder(
   const Database& database,
   std::vector<CopyOnWriteBitmap> bitmap_filter
) const {
   validateOrderByFields(database);

   // Hacky solution to give the full feature set (randomization,
   // sorting) for small result sets, and streaming without those
   // features for larger ones.

   size_t num_rows = 0;
   for (auto& bitmap : bitmap_filter) {
      num_rows += bitmap->cardinality();
   }
   const bool is_large = num_rows > MATERIALIZATION_CUTOFF;

   QueryResult result = execute(database, std::move(bitmap_filter));

   if (result.isMaterialized() || !is_large) {
      SPDLOG_TRACE("materialized or small -> full featured sort, offset+limit");
      result.materialize();
      if (offset.has_value() && offset.value() >= result.entriesMut().size()) {
         // Optimization: avoid sorting the result set, if the
         // offset+limit yields an empty result.
         return {};
      }
      applySort(result);
      applyOffsetAndLimit(result);
   } else {
      // Report an error if the unimplemented limit / offset or sort
      // features are relevant for the query.  Currently *assumes* that
      // none of the actions actually implement limit+offset
      auto error = [&](const std::string_view& what) {
         throw silo::QueryEvaluationException(fmt::format(
            "{} not supported for streaming endpoints when returning more than {} rows, but got "
            "{}",
            what,
            MATERIALIZATION_CUTOFF,
            num_rows
         ));
      };
      if (offset.has_value() && offset.value() > 0) {
         error("offset");
      }
      if (limit.has_value()) {
         error("limit");
      }
      if (!order_by_fields.empty()) {
         error("sorting");
      }
   }

   return result;
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void from_json(const nlohmann::json& json, OrderByField& field) {
   if (json.is_string()) {
      field = {.name = json.get<std::string>(), .ascending = true};
      return;
   }
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("field") && json.contains("order") &&
         json["field"].is_string() && json["order"].is_string(),
      "The orderByField '" + json.dump() +
         "' must be either a string or an object containing the fields 'field':string and "
         "'order':string, where the value of order is 'ascending' or 'descending'"
   );
   const std::string field_name = json["field"].get<std::string>();
   const std::string order_string = json["order"].get<std::string>();
   CHECK_SILO_QUERY(
      order_string == "ascending" || order_string == "descending",
      "The orderByField '" + json.dump() +
         "' must be either a string or an object containing the fields 'field':string and "
         "'order':string, where the value of order is 'ascending' or 'descending'"
   );
   field = {.name = field_name, .ascending = order_string == "ascending"};
}

std::optional<uint32_t> parseLimit(const nlohmann::json& json) {
   CHECK_SILO_QUERY(
      !json.contains("limit") || json["limit"].is_number_unsigned(),
      "If the action contains a limit, it must be a non-negative number"
   );
   return json.contains("limit") ? std::optional<uint32_t>(json["limit"].get<uint32_t>())
                                 : std::nullopt;
}

std::optional<uint32_t> parseOffset(const nlohmann::json& json) {
   CHECK_SILO_QUERY(
      !json.contains("offset") || json["offset"].is_number_unsigned(),
      "If the action contains an offset, it must be a non-negative number"
   );
   return json.contains("offset") ? std::optional<uint32_t>(json["offset"].get<uint32_t>())
                                  : std::nullopt;
}

std::optional<uint32_t> parseRandomizeSeed(const nlohmann::json& json) {
   if (!json.contains("randomize")) {
      return std::nullopt;
   }
   if (json["randomize"].is_boolean()) {
      if (json["randomize"].get<bool>()) {
         const uint32_t time_based_seed =
            std::chrono::system_clock::now().time_since_epoch().count();
         return time_based_seed;
      }
      return std::nullopt;
   }
   CHECK_SILO_QUERY(
      json["randomize"].is_object() && json["randomize"].contains("seed") &&
         json["randomize"]["seed"].is_number_unsigned(),
      "If the action contains 'randomize', it must be either a boolean or an object "
      "containing an unsigned 'seed'"
   );
   return json["randomize"]["seed"].get<uint32_t>();
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action) {
   CHECK_SILO_QUERY(json.contains("type"), "The field 'type' is required in any action");
   CHECK_SILO_QUERY(
      json["type"].is_string(),
      "The field 'type' in all actions needs to be a string, but is: " + json["type"].dump()
   );
   const std::string expression_type = json["type"];
   if (expression_type == "Aggregated") {
      action = json.get<std::unique_ptr<Aggregated>>();
   } else if (expression_type == "Mutations") {
      action = json.get<std::unique_ptr<Mutations<Nucleotide>>>();
   } else if (expression_type == "Details") {
      action = json.get<std::unique_ptr<Details>>();
   } else if (expression_type == "AminoAcidMutations") {
      action = json.get<std::unique_ptr<Mutations<AminoAcid>>>();
   } else if (expression_type == "Fasta") {
      action = json.get<std::unique_ptr<Fasta>>();
   } else if (expression_type == "FastaAligned") {
      action = json.get<std::unique_ptr<FastaAligned>>();
   } else if (expression_type == "Insertions") {
      action = json.get<std::unique_ptr<InsertionAggregation<Nucleotide>>>();
   } else if (expression_type == "AminoAcidInsertions") {
      action = json.get<std::unique_ptr<InsertionAggregation<AminoAcid>>>();
   } else {
      throw QueryParseException(expression_type + " is not a valid action");
   }

   CHECK_SILO_QUERY(
      !json.contains("orderByFields") || json["orderByFields"].is_array(),
      "orderByFields must be an array."
   );
   auto order_by_fields = json.contains("orderByFields")
                             ? json["orderByFields"].get<std::vector<OrderByField>>()
                             : std::vector<OrderByField>();
   CHECK_SILO_QUERY(
      !json.contains("offset") || json["offset"].is_number_unsigned(),
      "If the action contains an offset, it must be a non-negative number"
   );
   auto limit = parseLimit(json);
   auto offset = parseOffset(json);
   auto randomize_seed = parseRandomizeSeed(json);
   action->setOrdering(order_by_fields, limit, offset, randomize_seed);
}

}  // namespace silo::query_engine::actions

#include "silo/query_engine/actions/mutations.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <arrow/util/future.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/storage/column/sequence_column.h"

namespace silo::query_engine::actions {

template <typename SymbolType>
Mutations<SymbolType>::Mutations(
   std::vector<std::string>&& sequence_names,
   double min_proportion,
   std::vector<std::string_view>&& fields
)
    : sequence_names(std::move(sequence_names)),
      min_proportion(min_proportion),
      fields(std::move(fields)) {
   if (this->fields.empty()) {
      this->fields = std::vector<std::string_view>{VALID_FIELDS.begin(), VALID_FIELDS.end()};
   }
}

namespace {

const std::string SEQUENCE_NAMES_FIELD_NAME = "sequenceNames";
const std::string MIN_PROPORTION_FIELD_NAME = "minProportion";

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
void from_json(const nlohmann::json& json, std::unique_ptr<Mutations<SymbolType>>& action) {
   std::vector<std::string> sequence_names;
   if (json.contains(SEQUENCE_NAMES_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[SEQUENCE_NAMES_FIELD_NAME].is_array(),
         "Mutations action can have the field {} of type array of "
         "strings, but no other type",
         SEQUENCE_NAMES_FIELD_NAME
      );
      for (const auto& child : json[SEQUENCE_NAMES_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field {}"
            " of Mutations action must have type "
            "array, if present. Found: {}",
            SEQUENCE_NAMES_FIELD_NAME,
            child.dump()
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   }

   CHECK_SILO_QUERY(
      json.contains(MIN_PROPORTION_FIELD_NAME) && json[MIN_PROPORTION_FIELD_NAME].is_number(),
      "Mutations action must contain the field {0}"
      " of type number with limits [0.0, "
      "1.0]. Only mutations are returned if the proportion of sequences having this mutation, "
      "is at least {0}",
      MIN_PROPORTION_FIELD_NAME
   );
   const double min_proportion = json[MIN_PROPORTION_FIELD_NAME].get<double>();
   if (min_proportion < 0 || min_proportion > 1) {
      throw IllegalQueryException(
         "Invalid proportion: " + MIN_PROPORTION_FIELD_NAME + " must be in interval [0.0, 1.0]"
      );
   }

   std::vector<std::string_view> fields;
   if (json.contains("fields")) {
      CHECK_SILO_QUERY(
         json["fields"].is_array(),
         "The field 'fields' for a Mutations action must be an array of strings"
      );
      for (const auto& field_json : json["fields"]) {
         CHECK_SILO_QUERY(
            field_json.is_string(),
            "The field 'fields' for a Mutations action must be an array of strings"
         );
         const std::string field = field_json;
         auto iter =
            std::ranges::find_if(Mutations<SymbolType>::VALID_FIELDS, [&](const auto& valid_field) {
               return valid_field == field;
            });
         CHECK_SILO_QUERY(
            iter != Mutations<SymbolType>::VALID_FIELDS.end(),
            "The attribute 'fields' contains an invalid field '{}'. Valid fields are {}.",
            field,
            boost::join(
               std::vector<std::string>{
                  Mutations<SymbolType>::VALID_FIELDS.begin(),
                  Mutations<SymbolType>::VALID_FIELDS.end()
               },
               ", "
            )
         );
         fields.push_back(*iter);
      }
   }

   action = std::make_unique<Mutations<SymbolType>>(
      std::move(sequence_names), min_proportion, std::move(fields)
   );
}

template class Mutations<AminoAcid>;
template class Mutations<Nucleotide>;
// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<Mutations<AminoAcid>>& action
);
// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<Mutations<Nucleotide>>& action
);

}  // namespace silo::query_engine::actions

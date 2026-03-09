#include "silo/query_engine/actions/insertions.h"

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/container_hash/hash.hpp>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::actions {

template <typename SymbolType>
InsertionAggregation<SymbolType>::InsertionAggregation(std::vector<std::string>&& sequence_names)
    : sequence_names(std::move(sequence_names)) {}

namespace {

const std::string SEQUENCE_NAMES_FIELD_NAME = "sequenceNames";

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<SymbolType>>& action
) {
   std::vector<std::string> sequence_names;
   if (json.contains(SEQUENCE_NAMES_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[SEQUENCE_NAMES_FIELD_NAME].is_array(),
         "The field '{}' of the insertions action must be of type string or array, was {}",
         SEQUENCE_NAMES_FIELD_NAME,
         std::string(json[SEQUENCE_NAMES_FIELD_NAME].type_name())
      );
      for (const auto& child : json[SEQUENCE_NAMES_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field {} of the Insertions action must have type string or an "
            "array, if present. Found: {}",
            SEQUENCE_NAMES_FIELD_NAME,
            child.dump()
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   }

   action = std::make_unique<InsertionAggregation<SymbolType>>(std::move(sequence_names));
}

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<AminoAcid>>& action
);

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<Nucleotide>>& action
);

template class InsertionAggregation<Nucleotide>;
template class InsertionAggregation<AminoAcid>;

}  // namespace silo::query_engine::actions

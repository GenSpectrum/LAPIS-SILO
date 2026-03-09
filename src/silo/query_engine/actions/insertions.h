#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/result.h>
#include <arrow/status.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

template <typename SymbolType>
class InsertionAggregation : public Action {
   static constexpr std::string_view POSITION_FIELD_NAME = "position";
   static constexpr std::string_view INSERTED_SYMBOLS_FIELD_NAME = "insertedSymbols";
   static constexpr std::string_view INSERTION_FIELD_NAME = "insertion";
   static constexpr std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   static constexpr std::string_view COUNT_FIELD_NAME = "count";

  public:
   std::vector<std::string> sequence_names;

   explicit InsertionAggregation(std::vector<std::string>&& sequence_names);
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<SymbolType>>& action
);

}  // namespace silo::query_engine::actions

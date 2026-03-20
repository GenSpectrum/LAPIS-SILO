#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class FastaAligned : public Action {
  public:
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;

   explicit FastaAligned(
      std::vector<std::string>&& sequence_names,
      std::vector<std::string>&& additional_fields
   );
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions

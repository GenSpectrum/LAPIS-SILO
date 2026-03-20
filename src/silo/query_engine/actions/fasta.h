#pragma once

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"

#include "silo/schema/database_schema.h"

namespace silo::query_engine::actions {

class Fasta : public Action {
  public:
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;

   explicit Fasta(
      std::vector<std::string>&& sequence_names,
      std::vector<std::string>&& additional_fields
   )
       : sequence_names(std::move(sequence_names)),
         additional_fields(std::move(additional_fields)) {}
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action);

}  // namespace silo::query_engine::actions

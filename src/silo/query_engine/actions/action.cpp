#include "silo/query_engine/actions/action.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/aa_mutations.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/nuc_mutations.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo::query_engine::actions {

Action::Action() = default;

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
   } else if (expression_type == "AAMutations") {
      action = json.get<std::unique_ptr<AAMutations>>();
   } else if (expression_type == "Fasta") {
      action = json.get<std::unique_ptr<Fasta>>();
   } else if (expression_type == "FastaAligned") {
      action = json.get<std::unique_ptr<FastaAligned>>();
   } else {
      throw QueryParseException(expression_type + " is not a valid action");
   }
}

}  // namespace silo::query_engine::actions

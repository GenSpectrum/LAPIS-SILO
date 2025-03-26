#include "silo/query_engine/actions/fasta.h"

#if defined(__linux__)
#include <malloc.h>
#endif

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>
#include <memory>
#include <nlohmann/json.hpp>

#include "silo/common/numbers.h"
#include "silo/common/panic.h"
#include "silo/common/range.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"

using silo::common::add1;
using silo::common::Range;

namespace silo::query_engine::actions {

Fasta::Fasta(std::vector<std::string>&& sequence_names)
    : sequence_names(sequence_names) {}

void Fasta::validateOrderByFields(const schema::TableSchema& schema) const {
   const std::string& primary_key_field = schema.primary_key.name;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == primary_key_field ||
            std::ranges::find(sequence_names, field.name) != std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the Fasta action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      )
   }
}

QueryResult Fasta::execute(const Database& database, std::vector<CopyOnWriteBitmap> bitmap_filter)
   const {
   for (const std::string& sequence_name : sequence_names) {
      CHECK_SILO_QUERY(
         true,  // TODO database.table.schema.contains(sequence_name),
         "Database does not contain an unaligned sequence with name: '" + sequence_name + "'"
      )
   }
   SILO_UNIMPLEMENTED();
#if defined(__linux__)
   SPDLOG_INFO(
      "Fasta sequences generated for partition. Manually invoking malloc_trim() to give "
      "back memory to OS."
   );
   malloc_trim(0);
#endif
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") &&
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "Fasta action must have the field sequenceName of type string or an array of "
      "strings"
   );
   std::vector<std::string> sequence_names;
   if (json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "Fasta action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }
   action = std::make_unique<Fasta>(std::move(sequence_names));
}

}  // namespace silo::query_engine::actions

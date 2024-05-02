#include "silo/query_engine/query_result.h"

#include <spdlog/spdlog.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "silo_api/variant_json_serializer.h"

namespace silo::query_engine {

void QueryResult::clear() {
   query_result_chunk_.clear();
   get_chunk_ = [](std::vector<QueryResultEntry>& /*query_result_chunk*/) {};
   i_ = 0;
}

std::optional<std::reference_wrapper<const QueryResultEntry>> QueryResult::next() {
   auto siz = query_result_chunk_.size();
   SPDLOG_DEBUG(
      "next called, i = {}, is_materialized_ = {}, chunk size() = {}", i_, is_materialized_, siz
   );
   if (i_ >= siz) {
      SPDLOG_DEBUG("reached the end of last chunk");
      query_result_chunk_.clear();
      SPDLOG_DEBUG("cleared vector");
      get_chunk_(query_result_chunk_);
      i_ = 0;
      siz = query_result_chunk_.size();
      SPDLOG_DEBUG("returned from get_chunk_, chunk size() = {}", siz);
      if (siz == 0) {
         SPDLOG_DEBUG("returning {} from next");
         return {};
      }
   }
   const QueryResultEntry& ref = query_result_chunk_[i_];
   ++i_;  // can't overflow because i_ >= siz will always be true before.
   SPDLOG_DEBUG("returning ref from next");
   return {std::cref(ref)};
}

std::vector<QueryResultEntry>& QueryResult::entriesMut() {
   if (!is_materialized_) {
      std::cerr << "can't give access to entries vector for a QueryResult that is streamed\n"
                << std::flush;
      abort();
   }
   return query_result_chunk_;
}

const std::vector<QueryResultEntry>& QueryResult::entries() const {
   if (!is_materialized_) {
      throw std::runtime_error(
         "can't give access to entries vector for a QueryResult that is streamed"
      );
   }
   return query_result_chunk_;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry) {
   for (const auto& [field, value] : result_entry.fields) {
      if (value.has_value()) {
         json[field] = value.value();
      } else {
         json[field] = nlohmann::json();
      }
   }
}

}  // namespace silo::query_engine

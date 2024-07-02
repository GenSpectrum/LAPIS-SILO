#include "silo/query_engine/query_result.h"

#include <spdlog/spdlog.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "silo_api/variant_json_serializer.h"

namespace silo::query_engine {

void QueryResult::clear() {
   query_result_chunk_.clear();
   get_chunk_ = [](std::vector<QueryResultEntry>& /*query_result_chunk*/) {};
   iter_ = query_result_chunk_.begin();
}

const QueryResultEntry* QueryResult::next() {
   SPDLOG_TRACE(
      "DEBUG: next called, is_materialized_ = {}, chunk len = {}",
      is_materialized_,
      query_result_chunk_.size()
   );
   if (iter_ == query_result_chunk_.end()) {
      SPDLOG_TRACE("reached the end of last chunk");
      query_result_chunk_.clear();
      get_chunk_(query_result_chunk_);
      SPDLOG_DEBUG("DEBUG: returned from get_chunk_");
      iter_ = query_result_chunk_.begin();
      SPDLOG_TRACE("DEBUG: set iterator to begin");
      if (query_result_chunk_.empty()) {
         SPDLOG_TRACE("returning nullopt from next");
         return nullptr;
      }
   }
   const QueryResultEntry* ptr = &*iter_;
   ++iter_;
   SPDLOG_TRACE("returning ref from next");
   return ptr;
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

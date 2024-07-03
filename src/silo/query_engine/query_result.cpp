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
   auto chunk_size = query_result_chunk_.size();
   SPDLOG_TRACE(
      "next called, i = {}, is_materialized_ = {}, chunk size() = {}",
      i_,
      is_materialized_,
      chunk_size
   );
   if (i_ >= chunk_size) {
      SPDLOG_TRACE("reached the end of last chunk");
      query_result_chunk_.clear();
      get_chunk_(query_result_chunk_);
      i_ = 0;
      chunk_size = query_result_chunk_.size();
      SPDLOG_TRACE("returned from get_chunk_, chunk size() = {}", chunk_size);
      if (chunk_size == 0) {
         SPDLOG_TRACE("returning nullopt from next");
         return {};
      }
   }
   const QueryResultEntry& ref = query_result_chunk_[i_];
   ++i_;  // can't overflow because i_ < chunk_size (if it was >=
          // chunk_size, it was set to 0)
   SPDLOG_TRACE("returning ref from next");
   return {std::cref(ref)};
}

void QueryResult::materialize() {
   if (is_materialized_) {
      return;
   }

   std::vector<QueryResultEntry> tmp{};
   while (true) {
      get_chunk_(tmp);
      if (tmp.empty()) {
         break;
      }
      if (query_result_chunk_.empty()) {
         // This is only an optimization
         query_result_chunk_ = std::move(tmp);
      } else {
         query_result_chunk_.insert(
            query_result_chunk_.end(),
            std::make_move_iterator(tmp.begin()),
            std::make_move_iterator(tmp.end())
         );
         tmp.clear();
      }
   }

   is_materialized_ = true;
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

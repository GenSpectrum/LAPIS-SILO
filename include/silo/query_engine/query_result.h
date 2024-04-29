#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/json_value_type.h"

namespace silo::query_engine {

struct QueryResultEntry {
   std::map<std::string, common::JsonValueType> fields;
};

class QueryResult {
   // Temporary chunk of the query result in the case of streaming, or
   // the whole result in the case of eager query evaluation.
   std::vector<QueryResultEntry> query_result_chunk_;
   // Receives the cleared `query_result_chunk_` and fills in the next
   // batch of entries of the result set; it adds no entries iff the
   // end of the result set has been reached.
   std::function<void(std::vector<QueryResultEntry>&)> get_chunk_;
   // Iterating through query_result_chunk_.
   std::vector<QueryResultEntry>::const_iterator iter_;
   // Used for safety checks when asking to get all values via
   // `entries` or `entriesMut`.
   bool is_materialized_;

  public:
   /// For eager query evaluation.
   explicit QueryResult(std::vector<QueryResultEntry>&& query_result)
       : query_result_chunk_(std::move(query_result)),
         get_chunk_([](std::vector<QueryResultEntry>& /*query_result_chunk*/) {}),
         is_materialized_(true) {
      iter_ = query_result_chunk_.begin();
   }
   /// For streaming query evaluation.
   explicit QueryResult(std::function<void(std::vector<QueryResultEntry>&)>&& get_chunk)
       : get_chunk_(get_chunk),
         is_materialized_(false) {
      iter_ = query_result_chunk_.begin();
   }
   /// The empty result.
   QueryResult()
       : get_chunk_([](std::vector<QueryResultEntry>& /*query_result_chunk*/) {}),
         is_materialized_(true) {}

   // Moves
   QueryResult(QueryResult&& other) noexcept
       : query_result_chunk_(std::move(other.query_result_chunk_)),
         get_chunk_(std::move(other.get_chunk_)),
         iter_(other.iter_),
         is_materialized_(other.is_materialized_){};
   QueryResult& operator=(QueryResult&& other) noexcept {
      query_result_chunk_ = std::move(other.query_result_chunk_);
      get_chunk_ = std::move(other.get_chunk_);
      iter_ = other.iter_;
      is_materialized_ = other.is_materialized_;
      // No marker needed since all fields have one themselves.
      return *this;
   }
   // Copy, only needed for testing::Return in gtest's
   // include/gmock/gmock-actions.h
   QueryResult(QueryResult& other) noexcept = default;

   /// Make the result empty.
   void clear();

   /// Returns nullptr at the end of the result set. The returned
   /// pointer is borrowing and is only valid until the next `next`
   /// call.
   std::optional<std::reference_wrapper<const QueryResultEntry>> next();

   /// Mutable access to the vector for sorting purposes. Can only be
   /// called for materialized QueryResult:s. NOTE: this method
   /// should probably be removed in the future and the code structured
   /// to never need sorting after creation of the QueryResult.
   std::vector<QueryResultEntry>& entriesMut();

   /// Access to the vector for backwards compatibility purposes. Can
   /// only be called for materialized QueryResult:s. NOTE: this
   /// method should perhaps be removed in the future.
   [[nodiscard]] const std::vector<QueryResultEntry>& entries() const;

   /// NOTE: this method should perhaps not exist in the future.
   [[nodiscard]] bool isMaterialized() const { return is_materialized_; }
};

// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry);
// NOLINTEND(readability-identifier-naming)

}  // namespace silo::query_engine

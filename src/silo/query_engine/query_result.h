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

/// TODO(#758) This class will be removed after the transition to Arrow is finished
class QueryResult {
   // Temporary chunk of the query result in the case of streaming, or
   // the whole result in the case of eager query evaluation.
   std::vector<QueryResultEntry> query_result_chunk_;
   // Receives the cleared `query_result_chunk_` and fills in the next
   // batch of entries of the result set; it adds no entries iff the
   // end of the result set has been reached.
   std::function<void(std::vector<QueryResultEntry>&)> get_chunk_;
   // Iterating through query_result_chunk_.
   size_t i_;
   // Used for safety checks when asking to get all values via
   // `entries` or `entriesMut`.
   bool is_materialized_;

  protected:
   QueryResult(
      std::vector<QueryResultEntry>&& query_result_chunk,
      std::function<void(std::vector<QueryResultEntry>&)>&& get_chunk,
      bool is_materialized
   )
       : query_result_chunk_(std::move(query_result_chunk)),
         get_chunk_(std::move(get_chunk)),
         i_(0),
         is_materialized_(is_materialized) {}

  public:
   /// For eager query evaluation.
   static QueryResult fromVector(std::vector<QueryResultEntry>&& query_result);
   /// For streaming query evaluation.
   static QueryResult fromGenerator(std::function<void(std::vector<QueryResultEntry>&)>&& get_chunk
   );
   /// The empty result.
   QueryResult()
       : get_chunk_([](std::vector<QueryResultEntry>& /*query_result_chunk*/) {}),
         i_(0),
         is_materialized_(true) {}

   // Moves
   QueryResult(QueryResult&& other) noexcept
       : query_result_chunk_(std::move(other.query_result_chunk_)),
         get_chunk_(std::move(other.get_chunk_)),
         i_(other.i_),
         is_materialized_(other.is_materialized_){};
   QueryResult& operator=(QueryResult&& other) noexcept;
   // Copy, only needed for testing::Return in gtest's
   // include/gmock/gmock-actions.h
   QueryResult(QueryResult& other) noexcept = default;

   /// Make the result empty.
   void clear();

   /// Returns nullptr at the end of the result set. The returned
   /// pointer is borrowing and is only valid until the next `next`
   /// call.
   std::optional<std::reference_wrapper<const QueryResultEntry>> next();

   /// Change a streaming result to a materialized one: after
   /// returning, all results have been retrieved and are available
   /// via next() or entries(). Execution before running next() is
   /// leading to unspecified results!  NOOP on materialized results.
   void materialize();

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

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry);

}  // namespace silo::query_engine

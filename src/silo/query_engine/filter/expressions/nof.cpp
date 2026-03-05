#include "silo/query_engine/filter/expressions/nof.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/intersection.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/threshold.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/column/horizontal_coverage_index.h"
#include "silo/storage/table_partition.h"

namespace {

using Operator = silo::query_engine::filter::operators::Operator;
using OperatorVector = silo::query_engine::filter::operators::OperatorVector;
using Empty = silo::query_engine::filter::operators::Empty;
using Full = silo::query_engine::filter::operators::Full;
using Complement = silo::query_engine::filter::operators::Complement;
using Intersection = silo::query_engine::filter::operators::Intersection;
using Union = silo::query_engine::filter::operators::Union;
using Threshold = silo::query_engine::filter::operators::Threshold;

std::unique_ptr<Operator> handleTrivialCases(
   const int updated_number_of_matchers,
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   bool match_exactly,
   uint32_t sequence_count
) {
   const int child_operator_count =
      static_cast<int>(non_negated_child_operators.size() + negated_child_operators.size());

   if (updated_number_of_matchers > child_operator_count) {
      return std::make_unique<Empty>(sequence_count);
   }
   if (updated_number_of_matchers < 0) {
      if (match_exactly) {
         return std::make_unique<Empty>(sequence_count);
      }
      return std::make_unique<Full>(sequence_count);
   }
   if (updated_number_of_matchers == 0) {
      if (!match_exactly) {
         return std::make_unique<Full>(sequence_count);
      }
      /// Now we want to match exactly none
      if (child_operator_count == 0) {
         return std::make_unique<Full>(sequence_count);
      }
      if (child_operator_count == 1) {
         if (non_negated_child_operators.empty()) {
            return std::move(negated_child_operators[0]);
         }
         return std::make_unique<Complement>(
            std::move(non_negated_child_operators[0]), sequence_count
         );
      }
      /// To negate entire result Not(Union) => Intersection(Not(Non-negated),Not(Negated))
      /// equiv: Intersection(Negated, Non-Negated) or Not(Union(Non-negated)), if negated empty
      if (negated_child_operators.empty()) {
         auto union_ret =
            std::make_unique<Union>(std::move(non_negated_child_operators), sequence_count);
         return std::make_unique<Complement>(std::move(union_ret), sequence_count);
      }
      return std::make_unique<Intersection>(
         std::move(negated_child_operators), std::move(non_negated_child_operators), sequence_count
      );
   }
   if (updated_number_of_matchers == 1 && child_operator_count == 1) {
      if (negated_child_operators.empty()) {
         return std::move(non_negated_child_operators[0]);
      }
      return std::make_unique<Complement>(std::move(negated_child_operators[0]), sequence_count);
   }
   return nullptr;
}

std::unique_ptr<Operator> handleAndCase(
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   uint32_t sequence_count
) {
   if (non_negated_child_operators.empty()) {
      std::unique_ptr<Union> union_ret =
         std::make_unique<Union>(std::move(negated_child_operators), sequence_count);
      return std::make_unique<Complement>(std::move(union_ret), sequence_count);
   }
   return std::make_unique<Intersection>(
      std::move(non_negated_child_operators), std::move(negated_child_operators), sequence_count
   );
}

std::unique_ptr<Operator> handleOrCase(
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   uint32_t sequence_count
) {
   if (negated_child_operators.empty()) {
      return std::make_unique<Union>(std::move(non_negated_child_operators), sequence_count);
   }
   /// De'Morgan if at least one negated
   std::unique_ptr<Intersection> intersection_ret = std::make_unique<Intersection>(
      std::move(negated_child_operators), std::move(non_negated_child_operators), sequence_count
   );
   return std::make_unique<Complement>(std::move(intersection_ret), sequence_count);
}

std::unique_ptr<Operator> toOperator(
   const int updated_number_of_matchers,
   OperatorVector&& non_negated_child_operators,
   OperatorVector&& negated_child_operators,
   bool match_exactly,
   uint32_t sequence_count
) {
   auto tmp = handleTrivialCases(
      updated_number_of_matchers,
      non_negated_child_operators,
      negated_child_operators,
      match_exactly,
      sequence_count
   );
   if (tmp) {
      return tmp;
   }

   const int child_operator_count =
      static_cast<int>(non_negated_child_operators.size() + negated_child_operators.size());

   if (updated_number_of_matchers == child_operator_count) {
      return handleAndCase(non_negated_child_operators, negated_child_operators, sequence_count);
   }
   if (updated_number_of_matchers == 1 && !match_exactly) {
      return handleOrCase(non_negated_child_operators, negated_child_operators, sequence_count);
   }
   return std::make_unique<Threshold>(
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      updated_number_of_matchers,
      match_exactly,
      sequence_count
   );
}

template <typename SymbolType>
std::optional<std::string> getCommonSequenceName(
   const silo::query_engine::filter::expressions::ExpressionVector& children,
   const silo::schema::TableSchema& table_schema
) {
   using SymbolInSetT = silo::query_engine::filter::expressions::SymbolInSet<SymbolType>;
   std::optional<std::string> common_sequence_name;
   for (const auto& child : children) {
      const auto* sis = dynamic_cast<const SymbolInSetT*>(child.get());
      if (sis == nullptr) {
         return std::nullopt;
      }
      auto sequence_name_of_sis =
         silo::validateSequenceNameOrGetDefault<SymbolType>(sis->getSequenceName(), table_schema);
      if (!common_sequence_name.has_value()) {
         common_sequence_name = sequence_name_of_sis;
      } else if (sequence_name_of_sis != common_sequence_name) {
         return std::nullopt;
      }
   }
   return common_sequence_name;
}

template <typename SymbolType>
using PositionQuery = silo::storage::column::VerticalSequenceIndex<SymbolType>::PositionQuery;

template <typename SymbolType>
struct ReferencePositionData {
   uint32_t position;
   std::vector<typename SymbolType::Symbol> excluded_mutations;
};

template <typename SymbolType>
std::pair<std::vector<PositionQuery<SymbolType>>, std::vector<ReferencePositionData<SymbolType>>>
distributeChildrenSequenceQueries(
   const silo::query_engine::filter::expressions::ExpressionVector& children,
   const silo::storage::column::SequenceColumnPartition<SymbolType>& col
) {
   std::vector<PositionQuery<SymbolType>> pure_queries;
   std::vector<ReferencePositionData<SymbolType>> ref_queries;

   using SymbolInSetT = silo::query_engine::filter::expressions::SymbolInSet<SymbolType>;
   for (const auto& child : children) {
      const auto* sis = dynamic_cast<const SymbolInSetT*>(child.get());
      SILO_ASSERT(sis != nullptr);
      const uint32_t position = sis->getPositionIdx();
      const auto& symbols = sis->getSymbols();
      const auto local_ref = col.getLocalReferencePosition(position);

      // We skip the child if the missing symbol (N or X) is in the query
      const bool includes_missing =
         std::find(symbols.begin(), symbols.end(), SymbolType::SYMBOL_MISSING) != symbols.end();
      if (includes_missing) {
         continue;
      }

      const bool includes_reference =
         std::find(symbols.begin(), symbols.end(), local_ref) != symbols.end();

      if (!includes_reference) {
         // All queried symbols are stored as diffs → positive match directly from the index.
         pure_queries.push_back({position, symbols});
      } else {
         // The reference is in the query set (compileWithReference case).
         // Compute the excluded mutations (non-query, non-missing symbols).
         // Coverage bitmaps are fetched in batches below.
         std::vector<typename SymbolType::Symbol> excluded_mutations;
         for (const auto sym : SymbolType::SYMBOLS) {
            if (sym == SymbolType::SYMBOL_MISSING) {
               continue;
            }
            if (std::find(symbols.begin(), symbols.end(), sym) == symbols.end()) {
               excluded_mutations.push_back(sym);
            }
         }
         ref_queries.push_back({position, std::move(excluded_mutations)});
      }
   }
   return std::make_pair(std::move(pure_queries), std::move(ref_queries));
}

// If all children are SymbolInSet<SymbolType> for the same sequence and none of their symbol
// sets contain the missing symbol, we can run a single forward scan over the vertical index
// with the Threshold DP inlined, avoiding k separate binary searches.
//
// For positions where the local reference is in the query set (compileWithReference case),
// the match bitmap is: coverage_bitmap - getMatchingContainersAsBitmap(pos, excluded_mutations).
// These are handled outside the vertical index method (in the ref_queries loop below).
template <typename SymbolType>
std::optional<std::unique_ptr<Operator>> tryCompileSequenceNOf(
   const silo::query_engine::filter::expressions::ExpressionVector& children,
   int number_of_matchers,
   bool match_exactly,
   const silo::storage::Table& table,
   const silo::storage::TablePartition& table_partition
) {
   if (children.empty()) {
      return std::nullopt;
   }

   const auto maybe_sequence_name = getCommonSequenceName<SymbolType>(children, table.schema);
   if (!maybe_sequence_name.has_value()) {
      return std::nullopt;
   }
   const auto& valid_sequence_name = maybe_sequence_name.value();

   const auto& col =
      table_partition.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   auto [pure_queries, ref_queries] = distributeChildrenSequenceQueries<SymbolType>(children, col);

   std::sort(
      pure_queries.begin(),
      pure_queries.end(),
      [](const PositionQuery<SymbolType>& left, const PositionQuery<SymbolType>& right) {
         return left.position < right.position;
      }
   );
   std::sort(
      ref_queries.begin(),
      ref_queries.end(),
      [](const ReferencePositionData<SymbolType>& left,
         const ReferencePositionData<SymbolType>& right) { return left.position < right.position; }
   );

   const int k_total = static_cast<int>(children.size());
   const auto dp_size = match_exactly ? static_cast<uint32_t>(number_of_matchers) + 1
                                      : static_cast<uint32_t>(number_of_matchers);

   // Build the DP table from pure-mutation positions via a single forward scan.
   std::vector<roaring::Roaring> dp_table;
   if (!pure_queries.empty()) {
      dp_table = col.vertical_sequence_index.buildNOfDpTable(
         pure_queries, static_cast<uint32_t>(number_of_matchers), match_exactly, k_total
      );
   } else {
      dp_table.resize(dp_size);
   }

   // Apply reference positions to the DP table in batches.
   // Positions are processed window-by-window so that getCoverageBitmapForPositions is called
   // once per window instead of once per position, avoiding repeated full-sequence scans.
   // match = covered sequences that do NOT have any of the excluded mutations.
   constexpr size_t COVERAGE_BATCH_SIZE = 64;
   const int max_table_index = static_cast<int>(dp_size) - 1;
   size_t ref_idx = 0;
   while (ref_idx < ref_queries.size()) {
      const uint32_t window_start =
         (ref_queries[ref_idx].position / COVERAGE_BATCH_SIZE) * COVERAGE_BATCH_SIZE;
      const uint32_t window_end = window_start + COVERAGE_BATCH_SIZE;
      const auto coverage_bitmaps =
         col.horizontal_coverage_index.template getCoverageBitmapForPositions<COVERAGE_BATCH_SIZE>(
            window_start
         );

      while (ref_idx < ref_queries.size() && ref_queries[ref_idx].position < window_end) {
         const auto& ref = ref_queries[ref_idx];
         const auto& coverage = coverage_bitmaps[ref.position - window_start];
         auto excluded_bitmap = col.vertical_sequence_index.getMatchingContainersAsBitmap(
            ref.position, ref.excluded_mutations
         );
         const roaring::Roaring match = coverage - excluded_bitmap;
         // Apply the same lower-bound pruning as buildNOfDpTable: entries dp[j] where
         // j < (n - remaining_positions) can never reach the threshold regardless of future
         // matches, so they need not be updated.
         const int overall_query_idx = static_cast<int>(pure_queries.size() + ref_idx);
         const int lower_bound = std::max(0, number_of_matchers - k_total + overall_query_idx - 1);
         for (int j = std::min(max_table_index, overall_query_idx); j > lower_bound; --j) {
            dp_table[j] |= dp_table[j - 1] & match;
         }
         if (k_total - overall_query_idx > number_of_matchers - 1) {
            dp_table[0] |= match;
         }
         ++ref_idx;
      }
   }

   // Finalize the DP table to produce the result bitmap.
   roaring::Roaring result;
   if (match_exactly) {
      dp_table[number_of_matchers - 1] -= dp_table[number_of_matchers];
      result = std::move(dp_table[number_of_matchers - 1]);
   } else {
      result = std::move(dp_table.back());
   }

   return std::make_unique<silo::query_engine::filter::operators::IndexScan>(
      silo::query_engine::CopyOnWriteBitmap{std::move(result)}, table_partition.sequence_count
   );
}

}  // namespace

namespace silo::query_engine::filter::expressions {

NOf::NOf(ExpressionVector&& children, int number_of_matchers, bool match_exactly)
    : children(std::move(children)),
      number_of_matchers(number_of_matchers),
      match_exactly(match_exactly) {}

std::string NOf::toString() const {
   std::string res;
   if (match_exactly) {
      res = "[exactly-" + std::to_string(number_of_matchers) + "-of:";
   } else {
      res = "[" + std::to_string(number_of_matchers) + "-of:";
   }
   for (const auto& child : children) {
      res += child->toString();
      res += ", ";
   }
   res += "]";
   return res;
}

std::tuple<operators::OperatorVector, operators::OperatorVector, int> NOf::mapChildExpressions(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   operators::OperatorVector child_operators;
   child_operators.reserve(children.size());
   for (const auto& child_expression : children) {
      child_operators.push_back(child_expression->compile(table, table_partition));
   }

   operators::OperatorVector non_negated_child_operators;
   operators::OperatorVector negated_child_operators;
   int updated_number_of_matchers = number_of_matchers;

   for (auto& child_operator : child_operators) {
      if (child_operator->type() == operators::EMPTY) {
         continue;
      }
      if (child_operator->type() == operators::FULL) {
         updated_number_of_matchers--;
         continue;
      }
      if (child_operator->type() == operators::COMPLEMENT) {
         auto canceled_negation = operators::Operator::negate(std::move(child_operator));
         negated_child_operators.emplace_back(std::move(canceled_negation));
         continue;
      }
      non_negated_child_operators.push_back(std::move(child_operator));
   }
   return std::tuple<operators::OperatorVector, operators::OperatorVector, int>{
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      updated_number_of_matchers
   };
}

ExpressionVector NOf::rewriteChildren(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   ExpressionVector rewritten_children;
   rewritten_children.reserve(children.size());
   for (const auto& child : children) {
      rewritten_children.push_back(child->rewrite(table, table_partition, mode));
   }
   return rewritten_children;
}

std::unique_ptr<Expression> NOf::rewriteToNonExact(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   auto at_least_k = std::make_unique<NOf>(
      rewriteChildren(table, table_partition, mode),
      this->number_of_matchers,
      /*match_exactly=*/false
   );
   auto at_least_k_plus_one = std::make_unique<NOf>(
      rewriteChildren(table, table_partition, mode),
      this->number_of_matchers + 1,
      /*match_exactly=*/false
   );
   ExpressionVector and_children;
   and_children.push_back(std::move(at_least_k));
   and_children.push_back(std::make_unique<Negation>(std::move(at_least_k_plus_one)));
   return std::make_unique<And>(std::move(and_children));
}

std::unique_ptr<Expression> NOf::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode mode
) const {
   // We cannot easily map ambiguity modes through an exact NOf expression -> rewrite without exact
   if (mode != NONE && match_exactly && std::cmp_less(number_of_matchers, children.size())) {
      return rewriteToNonExact(table, table_partition, mode);
   }

   return std::make_unique<NOf>(
      rewriteChildren(table, table_partition, mode), number_of_matchers, match_exactly
   );
}

std::unique_ptr<operators::Operator> NOf::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   // Fast path: all children are SymbolInSet for the same sequence with only non-reference,
   // non-missing symbols → single forward scan over the vertical index with inlined DP.
   if (auto result = tryCompileSequenceNOf<Nucleotide>(
          children, number_of_matchers, match_exactly, table, table_partition
       )) {
      return std::move(*result);
   }
   if (auto result = tryCompileSequenceNOf<AminoAcid>(
          children, number_of_matchers, match_exactly, table, table_partition
       )) {
      return std::move(*result);
   }

   auto [non_negated_child_operators, negated_child_operators, updated_number_of_matchers] =
      mapChildExpressions(table, table_partition);

   return toOperator(
      updated_number_of_matchers,
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      match_exactly,
      table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NOf>& filter) {
   CHECK_SILO_QUERY(
      json.contains("children"), "The field 'children' is required in an N-Of expression"
   );
   CHECK_SILO_QUERY(
      json["children"].is_array(), "The field 'children' in an N-Of expression needs to be an array"
   );
   CHECK_SILO_QUERY(
      json.contains("numberOfMatchers"),
      "The field 'numberOfMatchers' is required in an N-Of expression"
   );
   CHECK_SILO_QUERY(
      json["numberOfMatchers"].is_number_unsigned(),
      "The field 'numberOfMatchers' in an N-Of expression needs to be an unsigned integer"
   );
   CHECK_SILO_QUERY(
      json.contains("matchExactly"), "The field 'matchExactly' is required in an N-Of expression"
   );
   CHECK_SILO_QUERY(
      json["matchExactly"].is_boolean(),
      "The field 'matchExactly' in an N-Of expression needs to be a boolean"
   );

   const uint32_t number_of_matchers = json["numberOfMatchers"];
   const bool match_exactly = json["matchExactly"];
   auto children = json["children"].get<ExpressionVector>();
   filter = std::make_unique<NOf>(std::move(children), number_of_matchers, match_exactly);
}

}  // namespace silo::query_engine::filter::expressions

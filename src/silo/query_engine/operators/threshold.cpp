#include "silo/query_engine/operators/threshold.h"

#include <roaring/roaring.hh>
#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Threshold::Threshold(
   std::vector<std::unique_ptr<Operator>>&& non_negated_children,
   std::vector<std::unique_ptr<Operator>>&& negated_children,
   unsigned int number_of_matchers,
   bool match_exactly,
   unsigned sequence_count
)
    : non_negated_children(std::move(non_negated_children)),
      negated_children(std::move(negated_children)),
      number_of_matchers(number_of_matchers),
      match_exactly(match_exactly),
      sequence_count(sequence_count) {}

Threshold::~Threshold() noexcept = default;

std::string Threshold::toString() const {
   std::string res;
   if (match_exactly) {
      res += "=";
   } else {
      res += ">=";
   }
   for (const auto& child : this->non_negated_children) {
      res += ", " + child->toString();
   }
   for (const auto& child : this->non_negated_children) {
      res += ", ! " + child->toString();
   }
   res += ")";
   return res;
}

Type Threshold::type() const {
   return THRESHOLD;
}

OperatorResult Threshold::evaluate() const {
   unsigned dp_table_size;
   if (this->match_exactly) {
      // We need to keep track of the ones that matched too many
      dp_table_size = number_of_matchers + 1;
   } else {
      dp_table_size = number_of_matchers;
   }
   std::vector<roaring::Roaring*> partition_bitmaps(dp_table_size);
   // Copy bitmap of first child if immutable, otherwise use it directly
   if (non_negated_children.empty()) {
      partition_bitmaps[0] = negated_children[0]->evaluate().getMutable();
   } else {
      partition_bitmaps[0] = non_negated_children[0]->evaluate().getMutable();
   }
   // Initialize all bitmaps. Delete them later.
   for (unsigned i = 1; i < dp_table_size; ++i) {
      partition_bitmaps[i] = new roaring::Roaring();
   }

   if (non_negated_children.empty()) {
      partition_bitmaps[0]->flip(0, sequence_count);
   }

   // NOLINTBEGIN(readability-identifier-length)
   const int max_table_index = static_cast<int>(dp_table_size - 1);
   const int non_negated_child_count = static_cast<int>(non_negated_children.size());
   const int negated_child_count = static_cast<int>(negated_children.size());
   const int n = static_cast<int>(number_of_matchers);  // The threshold
   const int k = static_cast<int>(
      non_negated_children.size() + negated_children.size()
   );  // Number of loop iterations

   for (int i = 1; i < non_negated_child_count; ++i) {
      auto bitmap = non_negated_children[i]->evaluate();
      // positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the
      // conjunction would return 0
      // positions lower than n - k + i - 1 are unable to affect the result, because only (k - i)
      // iterations are left
      for (int j = std::min(max_table_index, i); j > std::max(0, n - k + i - 1); --j) {
         *partition_bitmaps[j] |= *partition_bitmaps[j - 1] & *bitmap.getConst();
      }
      if (0 >= n - k + i - 1) {
         *partition_bitmaps[0] |= *bitmap.getConst();
      }
      bitmap.free();
   }

   // Only now iterate over negated children.
   // We need to skip the first element, if it is already taken as start-point.
   // We change the operator from 'and' to 'and_not' for the propagation and update the 0th bitmap
   // with the inverse of the negated bitmap
   // We hope the case of flipping does not occur, as 'k - i' might always be '< n - 1'
   // (Number of children left is less than the distance we need to cross to reach the result)
   const int took_first_offset = non_negated_children.empty() ? 1 : 0;
   for (int local_i = took_first_offset; local_i < negated_child_count; ++local_i) {
      auto bitmap = negated_children[local_i]->evaluate();
      const int i = local_i + non_negated_child_count;
      // positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the
      // conjunction would return 0
      // positions lower than n - k + i - 1 are unable to affect the result, because only (k - i)
      // iterations are left
      for (int j = std::min(max_table_index, i); j > std::max(0, n - k + i - 1); --j) {
         *partition_bitmaps[j] |= *partition_bitmaps[j - 1] - *bitmap.getConst();
      }
      if (k - i >= n - 1) {
         roaring::api::roaring_bitmap_or_inplace(
            &partition_bitmaps[0]->roaring,
            roaring::api::roaring_bitmap_flip(&bitmap.getConst()->roaring, 0, sequence_count)
         );
      }
      bitmap.free();
   }
   // NOLINTEND(readability-identifier-length)

   // Delete intermediate results
   for (unsigned i = 0; i < number_of_matchers - 1; ++i) {
      delete partition_bitmaps[i];
   }

   if (this->match_exactly) {
      // Because exact, we remove all that have too many
      *partition_bitmaps[number_of_matchers - 1] -= *partition_bitmaps[number_of_matchers];

      delete partition_bitmaps[number_of_matchers];

      return OperatorResult(partition_bitmaps[number_of_matchers - 1]);
   }
   return OperatorResult(partition_bitmaps.back());
}

}  // namespace silo::query_engine::operators

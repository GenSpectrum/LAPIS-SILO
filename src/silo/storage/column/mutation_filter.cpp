#include "silo/storage/column/mutation_filter.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_for_each.h>

namespace silo::storage::column {

void MutationFilter::addSliceIdx(
   uint32_t slice_length,
   uint32_t overlap_shift,
   uint32_t mutation_count,
   std::vector<genome_ids_t> genome_ids_per_slice
) {
   const SliceIdxParameters slice_idx_parameter{slice_length, overlap_shift};
   slice_indexes[slice_idx_parameter].emplace_back(std::move(genome_ids_per_slice), mutation_count);
}

void MutationFilter::finalize() {
   for (auto& slice_idx_bucket : slice_indexes) {
      slice_idx_parameters.push_back(slice_idx_bucket.first);
      std::sort(slice_idx_bucket.second.begin(), slice_idx_bucket.second.end());
   }
   std::sort(slice_idx_parameters.begin(), slice_idx_parameters.end(), std::greater<>());
}

std::optional<const roaring::Roaring*> MutationFilter::filter(
   std::pair<uint32_t, uint32_t> range,
   uint32_t query_mutation_count
) const {
   auto [start, end] = range;
   std::optional<const roaring::Roaring*> result = std::nullopt;
   for (auto slice_idx_parameter : slice_idx_parameters) {
      const auto [slice_length, overlap_shift] = slice_idx_parameter;
      const auto bucket_idx = start / overlap_shift;
      if (bucket_idx * overlap_shift + slice_length < end) {
         return result;
      }
      const auto& slice_idx_vec = slice_indexes.find(slice_idx_parameter)->second;
      if (slice_idx_vec.front().mutation_count > query_mutation_count) {
         continue;
      }
      auto slice_idx_it = std::lower_bound(
         slice_idx_vec.begin(),
         slice_idx_vec.end(),
         query_mutation_count,
         [](const auto& slice_idx, const uint32_t query_mutation_count) {
            return slice_idx.mutation_count <= query_mutation_count;
         }
      );
      assert(slice_idx_it != slice_idx_vec.begin());
      --slice_idx_it;
      result = slice_idx_it->get_bucket_genome_ids(bucket_idx);
   }
   return result;
}

size_t MutationFilter::computeSize() const {
   size_t result = 0;
   /*
   for (const auto& position : positions) {
      for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
         result += position.bitmaps.at(symbol).getSizeInBytes(false);
      }
   }
   */
   return result;
}

size_t MutationFilter::runOptimize() {
   std::atomic<size_t> count_true = 0;
   /*
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      tbb::parallel_for(0, 100, [](int j){
         tbb::parallel_for(0, 100, [](int k){
            printf("Hello World %d/%d/%d\n", i, j, k);
         });
      });
   });
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      for (auto position = local.begin(); position != local.end(); ++position) {
         for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
            if (positions[position].bitmaps[symbol].runOptimize()) {
               ++count_true;
            }
         }
      }
   });
   count_true += mutation_filter.runOptimize();
    */
   return count_true;
}

size_t MutationFilter::shrinkToFit() {
   std::atomic<size_t> saved = 0;
   /*
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      size_t local_saved = 0;
      for (auto position = local.begin(); position != local.end(); ++position) {
         for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
            local_saved += positions[position].bitmaps[symbol].shrinkToFit();
         }
      }
      saved += local_saved;
   });
   saved += mutation_filter.shrinkToFit();
    */
   return saved;
}

}  // namespace silo::storage::column

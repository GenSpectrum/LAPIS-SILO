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
      result = slice_idx_it->getBucketGenomeIds(bucket_idx);
   }
   return result;
}

size_t MutationFilter::computeSize() const {
   size_t result = 0;
   for (const auto& slice_idx_bucket : slice_indexes) {
      for (const auto& slice_idx : slice_idx_bucket.second) {
         for (const auto& genome_ids : slice_idx.genome_ids_per_slice) {
            result += genome_ids.getSizeInBytes(false);
         }
      }
   }
   return result;
}

size_t MutationFilter::runOptimize() {
   std::atomic<size_t> count_true = 0;
   tbb::parallel_for_each(slice_indexes.begin(), slice_indexes.end(), [&](auto& slice_idx_bucket) {
      tbb::parallel_for_each(
         slice_idx_bucket.second.begin(),
         slice_idx_bucket.second.end(),
         [&](auto& slice_idx) {
            tbb::parallel_for_each(
               slice_idx.genome_ids_per_slice.begin(),
               slice_idx.genome_ids_per_slice.end(),
               [&](auto& genome_ids) {
                  if (genome_ids.runOptimize()) {
                     ++count_true;
                  }
               }
            );
         }
      );
   });
   return count_true;
}

size_t MutationFilter::shrinkToFit() {
   std::atomic<size_t> saved = 0;
   tbb::parallel_for_each(slice_indexes.begin(), slice_indexes.end(), [&](auto& slice_idx_bucket) {
      tbb::parallel_for_each(
         slice_idx_bucket.second.begin(),
         slice_idx_bucket.second.end(),
         [&](auto& slice_idx) {
            tbb::parallel_for_each(
               slice_idx.genome_ids_per_slice.begin(),
               slice_idx.genome_ids_per_slice.end(),
               [&](auto& genome_ids) { saved += genome_ids.shrinkToFit(); }
            );
         }
      );
   });
   return saved;
}

}  // namespace silo::storage::column

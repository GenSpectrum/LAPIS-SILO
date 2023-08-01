#include "silo/storage/column/mutation_filter.h"

#include <gtest/gtest.h>
#include <boost/functional/hash.hpp>

using silo::storage::column::MutationFilter;

static constexpr uint32_t GENOME_LENGTH = 128;

struct SliceIdxBitmapParameters {
   uint32_t slice_length = 0;
   uint32_t mutation_count = 0;
   uint32_t range_start = 0;

   auto operator==(const auto other) const {
      return slice_length == other.slice_length && mutation_count == other.mutation_count &&
             range_start == other.range_start;
   };

   struct Hasher {
      std::size_t operator()(const SliceIdxBitmapParameters& bitmap_params) const noexcept {
         std::size_t seed = 0;
         boost::hash_combine(seed, bitmap_params.slice_length);
         boost::hash_combine(seed, bitmap_params.mutation_count);
         boost::hash_combine(seed, bitmap_params.range_start);
         return seed;
      }
   };
};

using SliceIdxBitmapStorage =
   std::unordered_map<SliceIdxBitmapParameters, roaring::Roaring, SliceIdxBitmapParameters::Hasher>;

static std::pair<MutationFilter, SliceIdxBitmapStorage> setupTest() {
   const auto start_length = 8;
   SliceIdxBitmapStorage truth;
   MutationFilter mutation_filter;

   for (uint32_t slice_length = start_length; slice_length <= GENOME_LENGTH; slice_length *= 2) {
      for (uint32_t div = start_length; div >= 1; div /= 2) {
         const auto mutation_count = slice_length / div;
         const auto overlap_shift = slice_length / 2;
         std::vector<roaring::Roaring> genome_ids_per_slice;
         for (uint32_t start_pos = 0; start_pos < GENOME_LENGTH; start_pos += overlap_shift) {
            const SliceIdxBitmapParameters bitmap_params{slice_length, mutation_count, start_pos};
            auto bitmap = roaring::Roaring{slice_length, mutation_count, start_pos};
            truth.emplace(bitmap_params, bitmap);
            genome_ids_per_slice.push_back(std::move(bitmap));
         }
         mutation_filter.addSliceIdx(
            slice_length, overlap_shift, mutation_count, std::move(genome_ids_per_slice)
         );
      }
   }
   mutation_filter.finalize();
   return std::make_pair(std::move(mutation_filter), std::move(truth));
}

TEST(MutationFilter, findCorrectSliceIdx) {
   auto [mutationFilter, truth] = setupTest();
   {
      const uint32_t query_mutation_count = 4;
      const std::pair<uint32_t, uint32_t> range{0, 32};

      const uint32_t slice_length = 32;
      const uint32_t slice_mutation_count = 4;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
   {
      const uint32_t query_mutation_count = 7;
      const std::pair<uint32_t, uint32_t> range{16, 48};

      const uint32_t slice_length = 32;
      const uint32_t slice_mutation_count = 4;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
   {
      const uint32_t query_mutation_count = 8;
      const std::pair<uint32_t, uint32_t> range{32, 64};

      const uint32_t slice_length = 32;
      const uint32_t slice_mutation_count = 8;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
   {
      const uint32_t query_mutation_count = GENOME_LENGTH / 8;
      const std::pair<uint32_t, uint32_t> range{0, GENOME_LENGTH};

      const uint32_t slice_length = GENOME_LENGTH;
      const uint32_t slice_mutation_count = GENOME_LENGTH / 8;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
   {
      const uint32_t query_mutation_count = GENOME_LENGTH;
      const std::pair<uint32_t, uint32_t> range{0, GENOME_LENGTH};

      const uint32_t slice_length = GENOME_LENGTH;
      const uint32_t slice_mutation_count = GENOME_LENGTH;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
}

TEST(MutationFilter, endIdxIsOutOfBounds) {
   auto [mutationFilter, truth] = setupTest();
   {
      const std::pair<uint32_t, uint32_t> range{0, GENOME_LENGTH + 1};
      const uint32_t query_mutation_count = 3;
      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_FALSE(res.has_value());
   }
   {
      const uint32_t query_mutation_count = 3;
      const std::pair<uint32_t, uint32_t> range{GENOME_LENGTH - 8, GENOME_LENGTH + 1};

      const uint32_t slice_length = 16;
      const uint32_t slice_mutation_count = 2;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
}

TEST(MutationFilter, queryMutationCountIsTooLow) {
   auto [mutationFilter, truth] = setupTest();
   {
      const std::pair<uint32_t, uint32_t> range{0, 8};
      const uint32_t query_mutation_count = 0;
      auto res = mutationFilter.filter(range, query_mutation_count);
      ASSERT_FALSE(res.has_value());
   }
   {
      for (uint32_t start_pos = 1; start_pos < 4; ++start_pos) {
         const std::pair<uint32_t, uint32_t> range{start_pos, start_pos + 8};
         const uint32_t query_mutation_count = 1;
         auto res = mutationFilter.filter(range, query_mutation_count);
         ASSERT_FALSE(res.has_value());
      }
      const uint32_t query_mutation_count = 1;
      const std::pair<uint32_t, uint32_t> range{4, 12};

      const uint32_t slice_length = 8;
      const uint32_t slice_mutation_count = 1;
      const SliceIdxBitmapParameters bitmap_params{slice_length, slice_mutation_count, range.first};

      auto res = mutationFilter.filter(range, query_mutation_count);

      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res.value(), truth.find(bitmap_params)->second);
   }
   {
      const std::pair<uint32_t, uint32_t> range{0, GENOME_LENGTH};
      const uint32_t query_mutation_count = GENOME_LENGTH / 16;
      auto res = mutationFilter.filter(range, query_mutation_count);
      ASSERT_FALSE(res.has_value());
   }
}

#ifndef SILO_MUTATION_FILTER_H
#define SILO_MUTATION_FILTER_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/functional/hash.hpp>
#include <roaring/roaring.hh>

namespace boost::serialization {
struct access;
}  // namespace boost::serialization

namespace silo::storage {
struct SequenceStorePartition;
}

namespace silo::storage::column {

class MutationFilter {
   friend class boost::serialization::access;

  public:
   using genome_ids_t = std::unique_ptr<roaring::Roaring>;
   using Iterator = roaring::Roaring::const_iterator;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive& slice_idx_parameters;
      archive& slice_indexes;
      // clang-format on
   }

   struct SliceIdxParameters {
      uint32_t slice_length;
      uint32_t overlap_shift;
      auto operator<=>(const SliceIdxParameters& other) const = default;

      struct Hasher {
         size_t operator()(const auto& slice_idx_parameters) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, slice_idx_parameters.slice_length);
            boost::hash_combine(seed, slice_idx_parameters.overlap_shift);
            return seed;
         }
      };
   };

   struct SliceIdx {
      std::vector<genome_ids_t> genome_ids_per_slice;
      size_t mutation_count;

      SliceIdx(std::vector<genome_ids_t> genome_ids_per_slice, size_t mutation_count)
          : genome_ids_per_slice(std::move(genome_ids_per_slice)),
            mutation_count(mutation_count) {}

      const roaring::Roaring* get_bucket_genome_ids(uint32_t bucket_idx) const {
         return genome_ids_per_slice[bucket_idx].get();
      }

      auto operator<=>(const SliceIdx& other) const {
         return mutation_count <=> other.mutation_count;
      };
   };

   std::vector<SliceIdxParameters> slice_idx_parameters;
   std::unordered_map<SliceIdxParameters, std::vector<SliceIdx>, SliceIdxParameters::Hasher>
      slice_indexes;

  public:
   void addSliceIdx(
      uint32_t slice_length,
      uint32_t overlap_shift,
      uint32_t mutation_count,
      std::vector<genome_ids_t> genome_ids_per_slice
   );

   void finalize();

   // Return all genome IDs that mutated at least *mutation_count* times between
   // the genome positions *range.first* and *range.second*
   std::optional<const roaring::Roaring*> filter(
      std::pair<uint32_t, uint32_t> range,
      uint32_t query_mutation_count
   ) const;

   size_t computeSize() const;

   size_t runOptimize();

   size_t shrinkToFit();
};

}  // namespace silo::storage::column

#endif  // SILO_MUTATION_FILTER_H

/*
 *
 *
 */
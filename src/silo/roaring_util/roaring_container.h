#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <roaring/roaring.hh>

namespace silo::roaring_util {

/// Owning RAII wrapper around a single roaring bitmap container (the 2^16-valued building block a
/// `roaring::Roaring` is internally composed of). It bundles the raw `roaring::internal` pointer,
/// its typecode and its cardinality, and encapsulates the container-level C API -- adding values,
/// sizing, run-optimizing, cloning and (de)serializing -- so callers do not reach into
/// `roaring::internal` directly.
class RoaringContainer {
   roaring::internal::container_t* container = nullptr;
   uint32_t cardinality = 0;
   uint8_t typecode = 0;

  public:
   /// Constructs an empty container that owns nothing (required for boost deserialization).
   /// Container will be in invalid state, rawContainer() must be initialized before usage
   /// Prefer `withCapacity`/`clonedFrom`/the owning constructor otherwise.
   RoaringContainer() = default;

   /// Takes ownership of an already-allocated container.
   RoaringContainer(
      roaring::internal::container_t* container,
      uint32_t cardinality,
      uint8_t typecode
   )
       : container(container),
         cardinality(cardinality),
         typecode(typecode) {}

   /// Creates an empty container sized for `capacity` elements
   static RoaringContainer withCapacity(int32_t capacity);

   /// Deep-copies an externally owned container and computes its cardinality.
   static RoaringContainer clonedFrom(
      const roaring::internal::container_t* source,
      uint8_t typecode
   );

   RoaringContainer(RoaringContainer&& other) noexcept
       : container(other.container),
         cardinality(other.cardinality),
         typecode(other.typecode) {
      other.container = nullptr;
   }
   RoaringContainer& operator=(RoaringContainer&& other) noexcept {
      if (this != &other) {
         std::swap(container, other.container);
         std::swap(cardinality, other.cardinality);
         std::swap(typecode, other.typecode);
      }
      return *this;
   }
   // Move-only to avoid double freeing of resources
   RoaringContainer(const RoaringContainer&) = delete;
   RoaringContainer& operator=(const RoaringContainer&) = delete;

   ~RoaringContainer() {
      if (container != nullptr) {
         roaring::internal::container_free(container, typecode);
      }
   }

   /// Adds a single 16-bit value, growing/reallocating the underlying container as needed. Like the
   /// roaring container API this blindly bumps the cardinality, so the caller must only add values
   /// that are not already present.
   void add(uint16_t value);

   [[nodiscard]] uint32_t getCardinality() const { return cardinality; }

   [[nodiscard]] bool empty() const { return cardinality == 0; }

   [[nodiscard]] size_t sizeInBytes() const;

   /// Converts the container to a run container if that is more compact, then shrinks it to fit.
   void runOptimizeAndShrink();

   /// Raw handle for interop with the roaring `internal` API (e.g. BitmapBuilderByContainer,
   /// roaringSubsetRanks). The container stays owned by this object.
   [[nodiscard]] const roaring::internal::container_t* rawContainer() const { return container; }

   [[nodiscard]] uint8_t getTypecode() const { return typecode; }

   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      if constexpr (Archive::is_saving::value) {
         // clang-format off
         archive & cardinality;
         archive & typecode;
         // clang-format on
         const size_t size_in_bytes =
            roaring::internal::container_size_in_bytes(container, typecode);
         std::string buffer(size_in_bytes, '\0');
         roaring::internal::container_write(container, typecode, buffer.data());
         archive << buffer;
      } else {
         // Free any container this object already owns before overwriting its bookkeeping, so
         // deserializing into a live container does not leak.
         if (container != nullptr) {
            roaring::internal::container_free(container, typecode);
            container = nullptr;
         }
         // clang-format off
         archive & cardinality;
         archive & typecode;
         // clang-format on
         std::string buffer;
         archive >> buffer;
         // The container-level read API takes the cardinality as a signed int32_t.
         const auto signed_cardinality = static_cast<int32_t>(cardinality);
         if (typecode == BITSET_CONTAINER_TYPE) {
            auto* bitset = roaring::internal::bitset_container_create();
            if (bitset == nullptr) {
               throw std::runtime_error("failed to allocate bitset container");
            }
            bitset_container_read(signed_cardinality, bitset, buffer.data());
            container = bitset;
         } else if (typecode == RUN_CONTAINER_TYPE) {
            auto* run = roaring::internal::run_container_create();
            if (run == nullptr) {
               throw std::runtime_error("failed to allocate run container");
            }
            run_container_read(signed_cardinality, run, buffer.data());
            container = run;
         } else if (typecode == ARRAY_CONTAINER_TYPE) {
            auto* array =
               roaring::internal::array_container_create_given_capacity(signed_cardinality);
            if (array == nullptr) {
               throw std::runtime_error("failed to allocate array container");
            }
            array_container_read(signed_cardinality, array, buffer.data());
            container = array;
         } else {
            throw std::runtime_error("unknown roaring container typecode");
         }
      }
   }
};

// On 64-bit systems we expect a 16 byte struct, on 32-bit a 12 byte struct
static_assert(sizeof(RoaringContainer) == (sizeof(void*) == 8 ? 16 : 12));

}  // namespace silo::roaring_util

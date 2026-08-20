#include "rhydb/roaring_util/roaring_container.h"

#include "rhydb/common/panic.h"

namespace rhydb::roaring_util {

RoaringContainer RoaringContainer::withCapacity(int32_t capacity) {
   roaring::internal::container_t* container;
   uint8_t typecode;
   // roaring::internal::DEFAULT_MAX_SIZE is the maximum size for array containers.
   if (capacity <= roaring::internal::DEFAULT_MAX_SIZE) {
      container = roaring::internal::array_container_create_given_capacity(capacity);
      typecode = ARRAY_CONTAINER_TYPE;
   } else {
      container = roaring::internal::bitset_container_create();
      typecode = BITSET_CONTAINER_TYPE;
   }
   SILO_ASSERT(container != nullptr);
   return RoaringContainer{container, 0, typecode};
}

RoaringContainer RoaringContainer::clonedFrom(
   const roaring::internal::container_t* source,
   uint8_t typecode
) {
   auto* clone = roaring::internal::container_clone(source, typecode);
   SILO_ASSERT(clone != nullptr);
   const auto cardinality =
      static_cast<uint32_t>(roaring::internal::container_get_cardinality(clone, typecode));
   return RoaringContainer{clone, cardinality, typecode};
}

void RoaringContainer::add(uint16_t value) {
   uint8_t new_typecode;
   auto* new_container =
      roaring::internal::container_add(container, value, typecode, &new_typecode);
   if (new_container != container) {
      roaring::internal::container_free(container, typecode);
      container = new_container;
      typecode = new_typecode;
   }
   cardinality += 1;
}

size_t RoaringContainer::sizeInBytes() const {
   return roaring::internal::container_size_in_bytes(container, typecode);
}

size_t RoaringContainerView::sizeInBytes() const {
   return roaring::internal::container_size_in_bytes(container, typecode);
}

void RoaringContainer::runOptimizeAndShrink() {
   uint8_t new_typecode;
   auto* new_container =
      roaring::internal::convert_run_optimize(container, typecode, &new_typecode);
   if (new_container != container) {
      container = new_container;
      typecode = new_typecode;
   }
   roaring::internal::container_shrink_to_fit(container, typecode);
}

namespace {
/// Wraps a freshly produced roaring container as an owning `RoaringContainer`, or frees it and
/// returns an empty container if it came out empty (roaring's binary container ops can yield an
/// empty container, which must not be kept -- see `RoaringContainer`'s "no empty containers"
/// invariant).
RoaringContainer ownContainer(roaring::internal::container_t* container, uint8_t typecode) {
   const auto cardinality =
      static_cast<uint32_t>(roaring::internal::container_get_cardinality(container, typecode));
   if (cardinality == 0) {
      roaring::internal::container_free(container, typecode);
      return {};
   }
   return RoaringContainer{container, cardinality, typecode};
}
}  // namespace

// The operation is sequenced into a local before `ownContainer` reads the out-param `typecode`,
// since the evaluation order of a call's arguments is unspecified.
RoaringContainer operator&(RoaringContainerView lhs, RoaringContainerView rhs) {
   if (lhs.empty() || rhs.empty()) {
      return {};
   }
   uint8_t typecode = 0;
   auto* result = roaring::internal::container_and(
      lhs.rawContainer(), lhs.getTypecode(), rhs.rawContainer(), rhs.getTypecode(), &typecode
   );
   return ownContainer(result, typecode);
}

RoaringContainer operator-(RoaringContainerView lhs, RoaringContainerView rhs) {
   if (lhs.empty()) {
      return {};
   }
   if (rhs.empty()) {
      return lhs.toOwning();
   }
   uint8_t typecode = 0;
   auto* result = roaring::internal::container_andnot(
      lhs.rawContainer(), lhs.getTypecode(), rhs.rawContainer(), rhs.getTypecode(), &typecode
   );
   return ownContainer(result, typecode);
}

RoaringContainer operator|(RoaringContainerView lhs, RoaringContainerView rhs) {
   if (lhs.empty()) {
      return rhs.empty() ? RoaringContainer{} : rhs.toOwning();
   }
   if (rhs.empty()) {
      return lhs.toOwning();
   }
   uint8_t typecode = 0;
   auto* result = roaring::internal::container_or(
      lhs.rawContainer(), lhs.getTypecode(), rhs.rawContainer(), rhs.getTypecode(), &typecode
   );
   return ownContainer(result, typecode);
}

RoaringContainer& operator|=(RoaringContainer& accumulator, RoaringContainerView addend) {
   accumulator = RoaringContainerView{accumulator} | addend;
   return accumulator;
}

}  // namespace rhydb::roaring_util

#pragma once

#include <vector>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <roaring/roaring.hh>

// no linting because needed by external library
// NOLINTBEGIN
BOOST_SERIALIZATION_SPLIT_FREE(::roaring::Roaring)
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const roaring::Roaring& bitmask,
   [[maybe_unused]] const uint32_t version
) {
   std::size_t expected_size_in_bytes = bitmask.getSizeInBytes();
   std::vector<char> buffer(expected_size_in_bytes);
   std::size_t size_in_bytes = bitmask.write(buffer.data());
   // clang-format off
   ar & size_in_bytes;
   ar & boost::serialization::make_binary_object(buffer.data(), size_in_bytes);
   // clang-format on
}

template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   roaring::Roaring& bitmask,
   [[maybe_unused]] const uint32_t version
) {
   std::size_t size_in_bytes = 0;
   // clang-format off
   ar & size_in_bytes;
   std::vector<char> buffer(size_in_bytes);
   ar & boost::serialization::make_binary_object(buffer.data(), size_in_bytes);
   // clang-format on
   bitmask = roaring::Roaring::readSafe(buffer.data(), size_in_bytes);
}
}  // namespace boost::serialization
// NOLINTEND

#pragma once

#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/library_version_type.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <optional>

namespace boost::serialization {
template <class Archive, class T>
void save(
   Archive& archive,
   const std::optional<T>& optional,
   const uint32_t /*version*/
) {
   const auto has_value = optional.has_value();
   archive << BOOST_SERIALIZATION_NVP(has_value);

   if (has_value) {
      const auto& value = optional.value();
      archive << BOOST_SERIALIZATION_NVP(value);
   }
}

template <class Archive, class T>
void load(Archive& archive, std::optional<T>& optional, const uint32_t /*version*/) {
   auto has_value = bool{};
   archive >> BOOST_SERIALIZATION_NVP(has_value);

   optional.reset();

   if (has_value) {
      T value;
      archive >> BOOST_SERIALIZATION_NVP(value);
      optional = std::move(value);
   }
}

template <class Archive, class T>
void serialize(Archive& archive, std::optional<T>& optional, const uint32_t version) {
   boost::serialization::split_free(archive, optional, version);
}

}  // namespace boost::serialization
